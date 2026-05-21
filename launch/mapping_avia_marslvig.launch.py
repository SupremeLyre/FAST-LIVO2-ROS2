import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_dir = get_package_share_directory("fast_livo")
    rviz = LaunchConfiguration("rviz")
    livox_msg_type = LaunchConfiguration("livox_msg_type")

    mapping_node = Node(
        package="fast_livo",
        executable="fastlivo_mapping",
        name="laserMapping",
        output="screen",
        parameters=[
            os.path.join(pkg_dir, "config", "MARS_LVIG.yaml"),
            os.path.join(pkg_dir, "config", "camera_MARS_LVIG.yaml"),
            {"common.livox_msg_type": livox_msg_type},
        ],
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz",
        output="screen",
        arguments=["-d", os.path.join(pkg_dir, "rviz_cfg", "M300.rviz")],
        condition=IfCondition(rviz),
    )

    republish_node = Node(
        package="image_transport",
        executable="republish",
        name="republish",
        output="screen",
        parameters=[{"in_transport": "compressed", "out_transport": "raw"}],
        remappings=[("in", "/left_camera/image"), ("out", "/left_camera/image")],
    )

    return LaunchDescription(
        [
            DeclareLaunchArgument("rviz", default_value="true"),
            DeclareLaunchArgument("livox_msg_type", default_value="livox_ros_driver"),
            mapping_node,
            rviz_node,
            republish_node,
        ]
    )
