#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <MQ135.h>
#include <Adafruit_Sensor.h>
#include "home.html"
#include "login.html"

#define DHT_PIN 15
#define DHTTYPE DHT11
// khai bao chan cam bien
DHT dht(DHT_PIN, DHTTYPE);
#define MQ_PIN 32
MQ135 mq135(MQ_PIN);
// khai bao chan thiet bi và trạng thái
bool isFanOn = false;
bool isLightOn = false;
const int FAN_RELAY_PIN = 4;
const int LIGHT_RELAY_PIN = 5;

bool isAuthenticated = false;
const char *adminUsername = "247";
const char *adminPassword = "240712";

const char *ssid = "Dang Thi Hoai";
const char *password = "Etzetkhong9";

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
// Khai báo biến lưu thời gian cập nhật dữ liệu
unsigned long lastUpdate = 0;
const int updateInterval = 30000; // Khoảng thời gian cập nhật (30 giây)
float fanThreshold = 35.0;        // ngưỡng
// đọc nhiệt độ
String readDHTTemperature()
{
  float t = dht.readTemperature();
  if (isnan(t))
  { // nếu là kí tự chữ
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else
  {
    Serial.println(t);
    return String(t);
  }
}
// đọc thông số độ ẩm
String readDHTHumidity()
{
  float h = dht.readHumidity();
  if (isnan(h))
  {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else
  {
    Serial.println(h);
    return String(h);
  }
}
void toggleFan()
{
  isFanOn = !isFanOn;
  digitalWrite(FAN_RELAY_PIN, isFanOn ? HIGH : LOW);
}

void toggleLight()
{
  isLightOn = !isLightOn;
  digitalWrite(LIGHT_RELAY_PIN, isLightOn ? HIGH : LOW);
}
// Replaces placeholder with DHT values
String processor(const String &var)
{
  if (var == "TEMPERATURE")
  {
    return readDHTTemperature();
  }
  else if (var == "HUMIDITY")
  {
    return readDHTHumidity();
  }
  else if (var == "FAN_STATUS")
  {
    return isFanOn ? "ON" : "OFF";
  }
  else if (var == "LIGHT_STATUS")
  {
    return isLightOn ? "ON" : "OFF";
  }

  return String();
}
void setup()
{
  // kết nối wifi
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
  // trạng thái chân đầu ra
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (!isAuthenticated) {
        request->send(200, "text/html", login_html);
    } else {
        request->redirect("/Home");
    } });

  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String username = request->arg("username");
    String password = request->arg("password");

    // Perform authentication logic here
    if (username.equals(adminUsername) && password.equals(adminPassword)) {
        isAuthenticated = true;
        request->redirect("/Home");
    } else {
        request->send(403, "text/plain", "Login failed");
    } });

  server.on("/Home", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (isAuthenticated) {
      float temp = dht.readTemperature();
      float hum=dht.readHumidity();
      float mqValue = mq135.getPPM();    
      String fanStatus = isFanOn ? "ON" : "OFF";
      request->send(200, "text/html", home_html);
    } else {
      request->send(401, "text/plain", "Unauthorized access");
    } });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", home_html, processor); });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readDHTTemperature().c_str()); });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readDHTHumidity().c_str()); });

  server.on("/fan-status", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", isFanOn ? "ON" : "OFF"); });
  server.on("/light-status", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", isLightOn ? "ON" : "OFF"); });
  server.on("/toggle-fan", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    toggleFan();
    request->send(200, "text/plain", "Fan status toggled"); });

  // Bắt đầu cập nhật dữ liệu cảm biến và broadcast qua WebSocket mỗi 30s
  setInterval(updateAndBroadcastSensorData, 30000);

  // Start server
  server.begin();
  webSocket.begin();
}

void loop()
{
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  float mqValue = mq135.getPPM();

  if (temp > fanThreshold)
  {
    digitalWrite(FAN_RELAY_PIN, HIGH);
    isFanOn = true; // Bật quạt thông qua relay
    // Serial.println("Fan is ON");
  }
  else
  {
    digitalWrite(FAN_RELAY_PIN, LOW);
    isFanOn = false; // Tắt quạt
    // Serial.println("Fan is OFF");
  }

  if (temp <= fanThreshold)
  {
    digitalWrite(LIGHT_RELAY_PIN, HIGH); // Bật đèn thông qua relay
    isLightOn = true;
  }
  else
  {
    digitalWrite(LIGHT_RELAY_PIN, LOW); // Tắt đèn
    isLightOn = false;
  }
  // Kiểm tra nếu đã đến thời gian cập nhật dữ liệu
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate >= updateInterval)
  {
    lastUpdate = currentMillis;

    // Cập nhật dữ liệu và broadcast qua WebSocket
    updateAndBroadcastSensorData();
  }

  webSocket.loop();
}
