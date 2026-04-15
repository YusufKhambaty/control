cmake_minimum_required(VERSION 3.8)
project(control_package)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find basic ROS 2 dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)

# Find the Raspberry Pi 'lgpio' hardware library
find_library(LGPIO_LIBRARIES lgpio REQUIRED)

# Step 1: Tell the compiler to turn your two .cpp files into a library
add_library(
  ${PROJECT_NAME} 
  SHARED
  src/pi_comms.cpp
  src/pi_system.cpp
)

# Step 2: Make the header files visible
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Step 3: Link the ROS 2 packages
ament_target_dependencies(
  ${PROJECT_NAME}
  rclcpp
  hardware_interface
  pluginlib
)

# Step 4: Link the Pi 5 GPIO library
target_link_libraries(${PROJECT_NAME} ${LGPIO_LIBRARIES})

# Step 5: Export the yellow pages plugin list we made
pluginlib_export_plugin_description_file(hardware_interface control_hardware.xml)

# Step 6: Install everything into the build directory
install(
  TARGETS ${PROJECT_NAME}
  DESTINATION lib
)

install(
  DIRECTORY include/ config/ launch/ urdf/
  DESTINATION share/${PROJECT_NAME}
)

install(
  FILES control_hardware.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(
  rclcpp
  hardware_interface
  pluginlib
)

ament_package()

