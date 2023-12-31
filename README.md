
cmake_minimum_required(VERSION 3.5)
project(vesc_driver)

# Force use of C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(vesc_msgs REQUIRED)
find_package(serial REQUIRED)

include_directories(
  include
)

# Create ROS 2 package
ament_package()

###########
## Build ##
###########

# driver libraries
add_library(vesc_driver SHARED
  src/vesc_driver.cpp
  src/vesc_interface.cpp
  src/vesc_packet.cpp
  src/vesc_packet_factory.cpp
)

# node executable
add_executable(vesc_driver_node src/vesc_driver_node.cpp)
target_link_libraries(vesc_driver_node
  vesc_driver
)

# nodelet library (ROS 2 doesn't use nodelets, but you can create a component if needed)
# add_library(vesc_driver_nodelet src/vesc_driver_nodelet.cpp)
# target_link_libraries(vesc_driver_nodelet
#   vesc_driver
# )

#############
## Install ##
#############

# Install targets
install(TARGETS
  vesc_driver
  vesc_driver_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install header files
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)

# Install launch files
install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)


# Install launch files
install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)
