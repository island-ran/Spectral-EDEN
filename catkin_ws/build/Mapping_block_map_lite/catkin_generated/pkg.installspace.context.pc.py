# generated from catkin/cmake/template/pkg.context.pc.in
CATKIN_PACKAGE_PREFIX = ""
PROJECT_PKG_CONFIG_INCLUDE_DIRS = "${prefix}/include".split(';') if "${prefix}/include" != "" else []
PROJECT_CATKIN_DEPENDS = "roscpp;tf;visualization_msgs;pcl_conversions;pcl_ros;cv_bridge;sensor_msgs;nav_msgs;geometry_msgs;swarm_data".replace(';', ' ')
PKG_CONFIG_LIBRARIES_WITH_PREFIX = "-lblock_map_lite_raycast;-lblock_map_lite_grid;-lblock_map_lite_color".split(';') if "-lblock_map_lite_raycast;-lblock_map_lite_grid;-lblock_map_lite_color" != "" else []
PROJECT_NAME = "block_map_lite"
PROJECT_SPACE_DIR = "/home/island/EDEN/catkin_ws/install"
PROJECT_VERSION = "0.0.0"
