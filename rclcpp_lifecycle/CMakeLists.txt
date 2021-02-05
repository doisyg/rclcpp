cmake_minimum_required(VERSION 3.5)

project(rclcpp_lifecycle)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wnon-virtual-dtor -Woverloaded-virtual)
endif()

find_package(ament_cmake_ros REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rcl_lifecycle REQUIRED)
find_package(rosidl_typesupport_cpp REQUIRED)
find_package(lifecycle_msgs REQUIRED)

### CPP High level library
add_library(rclcpp_lifecycle
  src/lifecycle_node.cpp
  src/node_interfaces/lifecycle_node_interface.cpp
  src/state.cpp
  src/transition.cpp
)
target_include_directories(${PROJECT_NAME}
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>")
# specific order: dependents before dependencies
ament_target_dependencies(rclcpp_lifecycle
  "rclcpp"
  "rcl_lifecycle"
  "lifecycle_msgs"
  "rosidl_typesupport_cpp"
)

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(rclcpp_lifecycle PRIVATE "RCLCPP_LIFECYCLE_BUILDING_DLL")

install(TARGETS
  rclcpp_lifecycle EXPORT rclcpp_lifecycle
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)


# specific order: dependents before dependencies
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_targets(${PROJECT_NAME})
ament_export_dependencies(rclcpp)
ament_export_dependencies(rcl_lifecycle)
ament_export_dependencies(lifecycle_msgs)
ament_package()

install(DIRECTORY include/
  DESTINATION include)
