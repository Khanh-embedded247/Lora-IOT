const express = require('express');
const path = require('path');
const app = express();

const PORT = 5500;

// Dùng để phục vụ các tệp tĩnh như CSS, JavaScript, v.v.
app.use(express.static('public')); // Thay 'public' bằng đường dẫn tới tệp tĩnh của bạn

// Định tuyến cho trang cá nhân của mỗi người dùng dựa trên tên người dùng
app.get('/personal/:username', (req, res) => {
    // Lấy tên người dùng từ URL
    const username = req.params.username;

    // Phục vụ tệp HTML trang cá nhân (ví dụ: personal.html)
    res.sendFile(path.join(__dirname, 'personal.html'));
});

app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});
