# generated from catkin/cmake/template/pkg.context.pc.in
CATKIN_PACKAGE_PREFIX = ""
PROJECT_PKG_CONFIG_INCLUDE_DIRS = "${prefix}/include".split(';') if "${prefix}/include" != "" else []
PROJECT_CATKIN_DEPENDS = "roscpp;visualization_msgs;sensor_msgs;geometry_msgs;nav_msgs;gcopter;swarm_exp_msgs;exp_comm_msgs;data_statistics".replace(';', ' ')
PKG_CONFIG_LIBRARIES_WITH_PREFIX = "-lswarm_data".split(';') if "-lswarm_data" != "" else []
PROJECT_NAME = "swarm_data"
PROJECT_SPACE_DIR = "/home/island/EDEN/catkin_ws/install"
PROJECT_VERSION = "0.0.0"
