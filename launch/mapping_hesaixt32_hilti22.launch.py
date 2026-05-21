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

    mapping_node = Node(
        package="fast_livo",
        executable="fastlivo_mapping",
        name="laserMapping",
        output="screen",
        parameters=[
            os.path.join(pkg_dir, "config", "HILTI22.yaml"),
            os.path.join(pkg_dir, "config", "camera_fisheye_HILTI22.yaml"),
        ],
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz",
        output="screen",
        arguments=["-d", os.path.join(pkg_dir, "rviz_cfg", "hilti.rviz")],
        condition=IfCondition(rviz),
    )

    return LaunchDescription(
        [
            DeclareLaunchArgument("rviz", default_value="true"),
            mapping_node,
            rviz_node,
        ]
    )
