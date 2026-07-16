# Install script for directory: /home/island/EDEN/catkin_ws/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/island/EDEN/catkin_ws/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  
      if (NOT EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
        file(MAKE_DIRECTORY "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
      endif()
      if (NOT EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/.catkin")
        file(WRITE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/.catkin" "")
      endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/island/EDEN/catkin_ws/install/_setup_util.py")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/island/EDEN/catkin_ws/install" TYPE PROGRAM FILES "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/_setup_util.py")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/island/EDEN/catkin_ws/install/env.sh")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/island/EDEN/catkin_ws/install" TYPE PROGRAM FILES "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/env.sh")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/island/EDEN/catkin_ws/install/setup.bash;/home/island/EDEN/catkin_ws/install/local_setup.bash")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/island/EDEN/catkin_ws/install" TYPE FILE FILES
    "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/setup.bash"
    "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/local_setup.bash"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/island/EDEN/catkin_ws/install/setup.sh;/home/island/EDEN/catkin_ws/install/local_setup.sh")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/island/EDEN/catkin_ws/install" TYPE FILE FILES
    "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/setup.sh"
    "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/local_setup.sh"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/island/EDEN/catkin_ws/install/setup.zsh;/home/island/EDEN/catkin_ws/install/local_setup.zsh")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/island/EDEN/catkin_ws/install" TYPE FILE FILES
    "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/setup.zsh"
    "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/local_setup.zsh"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/island/EDEN/catkin_ws/install/setup.fish;/home/island/EDEN/catkin_ws/install/local_setup.fish")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/island/EDEN/catkin_ws/install" TYPE FILE FILES
    "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/setup.fish"
    "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/local_setup.fish"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/island/EDEN/catkin_ws/install/.rosinstall")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/island/EDEN/catkin_ws/install" TYPE FILE FILES "/home/island/EDEN/catkin_ws/build/catkin_generated/installspace/.rosinstall")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/island/EDEN/catkin_ws/build/gtest/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/catkin_simple/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/gflags_catkin/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/glog_catkin/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Tools_kdtree/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/utils_quadrotor_msgs/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/stub_svo_msgs/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/stub_NKPlanner/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/utils_cmake_utils/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/stub_explore_con/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/stub_frontier_manager/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/stub_octomap_world/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/poscmd_2_odom/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/utils_pose_utils/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Trajectory_gcopter/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Trajectory_gcopter_debug/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/camera_sensing_mesh_render/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/so3_disturbance_generator/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/MSGS_swarm_exp_msgs/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/MSGS_exp_comm_msgs/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/data_statistics/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/utils_odom_visualization/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Mapping_map_visualization/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/so3_control/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Communication_swarm_data/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Mapping_block_map_lite/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Mapping_lowres_map_lite/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Mapping_eroi/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Mapping_mr_dtg_plus/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Exploration_eden/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/utils_multi_map_server/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/utils_uav_utils/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/so3_quadrotor_simulator/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/utils_rviz_plugins/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/stub_vikit_ros/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/camera_sensing_pointcloud_render/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/map_render/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/utils_waypoint_generator/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Trajectory_yaw_planner/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Test_ato_test/cmake_install.cmake")
  include("/home/island/EDEN/catkin_ws/build/Trajectory_traj_exc/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/island/EDEN/catkin_ws/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
