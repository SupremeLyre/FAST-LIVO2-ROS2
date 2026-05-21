cmake_minimum_required(VERSION 3.8)
project(fast_livo)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fexceptions")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")

message(STATUS "Current CPU architecture: ${CMAKE_SYSTEM_PROCESSOR}")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64|ARM|AARCH64)")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -mcpu=native -mtune=native -ffast-math")
    message(STATUS "Using 64-bit ARM optimizations: -O3 -mcpu=native -mtune=native -ffast-math")
  else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -mcpu=native -mtune=native -mfpu=neon -ffast-math")
    message(STATUS "Using 32-bit ARM optimizations: -O3 -mcpu=native -mtune=native -mfpu=neon -ffast-math")
  endif()
  add_definitions(-DARM_ARCH)
else()
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native -funroll-loops")
  message(STATUS "Using general x86 optimizations: -O3 -march=native -mtune=native -funroll-loops")
  add_definitions(-DX86_ARCH)
endif()

add_definitions(-DROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/\")

include(ProcessorCount)
ProcessorCount(N)
message(STATUS "Processor count: ${N}")
if(N GREATER 4)
  math(EXPR PROC_NUM "4")
  add_definitions(-DMP_EN -DMP_PROC_NUM=${PROC_NUM})
  message(STATUS "Multithreading enabled. Cores: ${PROC_NUM}")
elseif(N GREATER 1)
  math(EXPR PROC_NUM "${N}")
  add_definitions(-DMP_EN -DMP_PROC_NUM=${PROC_NUM})
  message(STATUS "Multithreading enabled. Cores: ${PROC_NUM}")
else()
  add_definitions(-DMP_PROC_NUM=1)
  message(STATUS "Single core detected. Multithreading disabled.")
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(livox_interfaces REQUIRED)
find_package(livox_ros_driver REQUIRED)
find_package(vikit_common REQUIRED)
find_package(vikit_ros REQUIRED)

find_package(Eigen3 REQUIRED)
find_package(PCL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Sophus REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)
find_package(OpenMP QUIET)
find_package(mimalloc QUIET)

if(OpenMP_CXX_FOUND)
  message(STATUS "OpenMP found")
else()
  message(STATUS "OpenMP not found, proceeding without it")
endif()

if(mimalloc_FOUND)
  message(STATUS "mimalloc found")
else()
  message(STATUS "mimalloc not found, proceeding without it")
endif()

set(AMENT_DEPS
  rclcpp
  geometry_msgs
  nav_msgs
  sensor_msgs
  std_msgs
  visualization_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
  pcl_conversions
  cv_bridge
  image_transport
  livox_interfaces
  livox_ros_driver
  vikit_common
  vikit_ros
)

set(FAST_LIVO_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIR}
  ${PCL_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${Sophus_INCLUDE_DIRS}
)

link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_library(vio STATIC src/vio.cpp src/frame.cpp src/visual_point.cpp)
add_library(lio STATIC src/voxel_map.cpp)
add_library(pre STATIC src/preprocess.cpp)
add_library(imu_proc STATIC src/IMU_Processing.cpp)
add_library(laser_mapping STATIC src/LIVMapper.cpp)

foreach(target vio lio pre imu_proc laser_mapping)
  target_include_directories(${target} PUBLIC ${FAST_LIVO_INCLUDE_DIRS})
  ament_target_dependencies(${target} ${AMENT_DEPS})
  target_link_libraries(${target}
    ${PCL_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${Sophus_LIBRARIES}
    ${Boost_LIBRARIES}
  )
  if(OpenMP_CXX_FOUND)
    target_link_libraries(${target} OpenMP::OpenMP_CXX)
  endif()
endforeach()

add_executable(fastlivo_mapping src/main.cpp)
target_include_directories(fastlivo_mapping PUBLIC ${FAST_LIVO_INCLUDE_DIRS})
ament_target_dependencies(fastlivo_mapping ${AMENT_DEPS})
target_link_libraries(fastlivo_mapping
  laser_mapping
  vio
  lio
  pre
  imu_proc
  ${PCL_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Sophus_LIBRARIES}
  ${Boost_LIBRARIES}
)

if(OpenMP_CXX_FOUND)
  target_link_libraries(fastlivo_mapping OpenMP::OpenMP_CXX)
endif()

if(mimalloc_FOUND)
  target_link_libraries(fastlivo_mapping mimalloc)
endif()

install(TARGETS fastlivo_mapping
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY config launch rviz_cfg
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
