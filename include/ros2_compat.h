#ifndef FAST_LIVO_ROS2_COMPAT_H_
#define FAST_LIVO_ROS2_COMPAT_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include <builtin_interfaces/msg/duration.hpp>
#include <builtin_interfaces/msg/time.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <livox_interfaces/msg/custom_msg.hpp>
#include <livox_interfaces/msg/custom_point.hpp>
#include <livox_ros_driver/msg/custom_msg.hpp>
#include <livox_ros_driver/msg/custom_point.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <nav_msgs/msg/path.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <std_msgs/msg/header.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <visualization_msgs/msg/marker.hpp>
#include <visualization_msgs/msg/marker_array.hpp>

namespace sensor_msgs
{
using Image = msg::Image;
using ImageConstPtr = msg::Image::ConstSharedPtr;
using Imu = msg::Imu;
using ImuConstPtr = msg::Imu::ConstSharedPtr;
using PointCloud2 = msg::PointCloud2;
} // namespace sensor_msgs

namespace nav_msgs
{
using Odometry = msg::Odometry;
using Path = msg::Path;
} // namespace nav_msgs

namespace geometry_msgs
{
using PoseStamped = msg::PoseStamped;
using Quaternion = msg::Quaternion;
using TransformStamped = msg::TransformStamped;
} // namespace geometry_msgs

namespace visualization_msgs
{
using Marker = msg::Marker;
using MarkerArray = msg::MarkerArray;
} // namespace visualization_msgs

namespace livox_ros_driver
{
using CustomMsg = msg::CustomMsg;
using CustomPoint = msg::CustomPoint;
} // namespace livox_ros_driver

namespace fast_livo
{
inline double stampToSec(const builtin_interfaces::msg::Time &stamp)
{
  return rclcpp::Time(stamp).seconds();
}

inline builtin_interfaces::msg::Time stampFromSec(double seconds)
{
  if (seconds < 0.0)
  {
    seconds = 0.0;
  }

  const double sec_floor = std::floor(seconds);
  builtin_interfaces::msg::Time stamp;
  stamp.sec = static_cast<int32_t>(sec_floor);
  stamp.nanosec = static_cast<uint32_t>(std::llround((seconds - sec_floor) * 1e9));
  if (stamp.nanosec >= 1000000000u)
  {
    stamp.sec += 1;
    stamp.nanosec -= 1000000000u;
  }
  return stamp;
}

inline builtin_interfaces::msg::Duration durationFromSec(double seconds)
{
  if (seconds < 0.0)
  {
    seconds = 0.0;
  }

  const double sec_floor = std::floor(seconds);
  builtin_interfaces::msg::Duration duration;
  duration.sec = static_cast<int32_t>(sec_floor);
  duration.nanosec = static_cast<uint32_t>(std::llround((seconds - sec_floor) * 1e9));
  if (duration.nanosec >= 1000000000u)
  {
    duration.sec += 1;
    duration.nanosec -= 1000000000u;
  }
  return duration;
}

inline std::string normalizeParameterName(const rclcpp::Node::SharedPtr &node, std::string name)
{
  while (!name.empty() && name.front() == '/')
  {
    name.erase(name.begin());
  }

  const std::string node_name = node ? node->get_name() : std::string();
  const std::string slash_prefix = node_name + "/";
  const std::string dot_prefix = node_name + ".";
  if (!node_name.empty() && name.rfind(slash_prefix, 0) == 0)
  {
    name.erase(0, slash_prefix.size());
  }
  else if (!node_name.empty() && name.rfind(dot_prefix, 0) == 0)
  {
    name.erase(0, dot_prefix.size());
  }

  std::replace(name.begin(), name.end(), '/', '.');
  return name;
}

template <typename T>
T getParameter(const rclcpp::Node::SharedPtr &node, const std::string &name, const T &default_value)
{
  const std::string param_name = normalizeParameterName(node, name);
  if (!node->has_parameter(param_name))
  {
    try
    {
      node->declare_parameter<T>(param_name, default_value);
    }
    catch (const rclcpp::exceptions::ParameterAlreadyDeclaredException &)
    {
    }
  }

  T value = default_value;
  node->get_parameter(param_name, value);
  return value;
}

template <typename T>
void getParameter(const rclcpp::Node::SharedPtr &node, const std::string &name, T &value, const T &default_value)
{
  value = getParameter<T>(node, name, default_value);
}

inline std::vector<int> getParameter(const rclcpp::Node::SharedPtr &node, const std::string &name, const std::vector<int> &default_value)
{
  std::vector<int64_t> default_value64(default_value.begin(), default_value.end());
  const std::vector<int64_t> value64 = getParameter<std::vector<int64_t>>(node, name, default_value64);
  return std::vector<int>(value64.begin(), value64.end());
}

inline void getParameter(
    const rclcpp::Node::SharedPtr &node, const std::string &name, std::vector<int> &value, const std::vector<int> &default_value)
{
  value = getParameter(node, name, default_value);
}
} // namespace fast_livo

namespace tf
{
inline geometry_msgs::Quaternion createQuaternionMsgFromRollPitchYaw(double roll, double pitch, double yaw)
{
  tf2::Quaternion q;
  q.setRPY(roll, pitch, yaw);
  return tf2::toMsg(q);
}
} // namespace tf

#ifndef ROS_INFO
#define ROS_INFO(...) RCLCPP_INFO(rclcpp::get_logger("fast_livo"), __VA_ARGS__)
#endif
#ifndef ROS_WARN
#define ROS_WARN(...) RCLCPP_WARN(rclcpp::get_logger("fast_livo"), __VA_ARGS__)
#endif
#ifndef ROS_ERROR
#define ROS_ERROR(...) RCLCPP_ERROR(rclcpp::get_logger("fast_livo"), __VA_ARGS__)
#endif
#ifndef ROS_ASSERT
#define ROS_ASSERT(cond) assert(cond)
#endif
#ifndef ROS_INFO_STREAM
#define ROS_INFO_STREAM(args)                                                                       \
  do                                                                                                \
  {                                                                                                 \
    std::ostringstream _fast_livo_stream;                                                           \
    _fast_livo_stream << args;                                                                      \
    RCLCPP_INFO(rclcpp::get_logger("fast_livo"), "%s", _fast_livo_stream.str().c_str());           \
  } while (0)
#endif
#ifndef ROS_WARN_STREAM
#define ROS_WARN_STREAM(args)                                                                       \
  do                                                                                                \
  {                                                                                                 \
    std::ostringstream _fast_livo_stream;                                                           \
    _fast_livo_stream << args;                                                                      \
    RCLCPP_WARN(rclcpp::get_logger("fast_livo"), "%s", _fast_livo_stream.str().c_str());           \
  } while (0)
#endif
#ifndef ROS_ERROR_STREAM
#define ROS_ERROR_STREAM(args)                                                                      \
  do                                                                                                \
  {                                                                                                 \
    std::ostringstream _fast_livo_stream;                                                           \
    _fast_livo_stream << args;                                                                      \
    RCLCPP_ERROR(rclcpp::get_logger("fast_livo"), "%s", _fast_livo_stream.str().c_str());          \
  } while (0)
#endif

#endif // FAST_LIVO_ROS2_COMPAT_H_
