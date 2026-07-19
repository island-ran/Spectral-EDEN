cmake_minimum_required(VERSION 3.0.2)
project(mr_dtg_plus)

add_definitions(-std=c++14)
set(CMAKE_BUILD_TYPE Release)

find_package(catkin_simple REQUIRED)
catkin_simple(ALL_DEPS_REQUIRED)

include_directories(
  ${catkin_INCLUDE_DIRS}
)

cs_add_library(${PROJECT_NAME}
  src/mr_dtg_plus.cpp
  src/mr_dtg_plus_global.cpp
  src/mr_dtg_plus_eden_baseline.cpp
  src/mr_dtg_plus_v4_global.cpp
  src/mr_dtg_plus_path_search.cpp
  src/mr_dtg_plus_initialization.cpp
  src/mr_dtg_plus_target_refine.cpp
  src/spectral_router.cpp
  src/spectral_worker.cpp
  src/committed_target_manager.cpp
  src/incremental_fiedler.cpp
  src/nystrom_bisection.cpp
  src/local_appr.cpp)

##########
# TESTS  #
##########
if(CATKIN_ENABLE_TESTING)
  catkin_add_gtest(spectral_router_test
    test/spectral_router_test.cpp)
  if(TARGET spectral_router_test)
    target_link_libraries(spectral_router_test
      ${PROJECT_NAME}
      gtest_main)
  endif()

  catkin_add_gtest(spectral_iterative_solver_test
    test/spectral_iterative_solver_test.cpp)
  if(TARGET spectral_iterative_solver_test)
    target_link_libraries(spectral_iterative_solver_test
      ${PROJECT_NAME}
      gtest_main)
  endif()

  catkin_add_gtest(committed_target_manager_test
    test/committed_target_manager_test.cpp)
  if(TARGET committed_target_manager_test)
    target_link_libraries(committed_target_manager_test
      ${PROJECT_NAME}
      gtest_main)
  endif()

  catkin_add_gtest(spectral_v4_backends_test
    test/spectral_v4_backends_test.cpp)
  if(TARGET spectral_v4_backends_test)
    target_link_libraries(spectral_v4_backends_test
      ${PROJECT_NAME}
      gtest_main)
  endif()

  catkin_add_gtest(spectral_worker_test
    test/spectral_worker_test.cpp)
  if(TARGET spectral_worker_test)
    target_link_libraries(spectral_worker_test
      ${PROJECT_NAME}
      gtest_main)
  endif()
endif()

##########
# EXPORT #
##########
cs_install()
cs_export()
