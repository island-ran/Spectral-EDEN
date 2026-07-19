cmake_minimum_required(VERSION 2.8.3)
project(eden)
set(CMAKE_BUILD_TYPE Release)

find_package(catkin_simple REQUIRED)
add_definitions(-std=c++14)

catkin_simple()


include_directories(
  ${catkin_INCLUDE_DIRS}
)
cs_add_library(${PROJECT_NAME}_lib src/eden.cpp src/eden_fsm.cpp)

cs_add_executable(${PROJECT_NAME}_node src/eden_node.cpp)

target_link_libraries(${PROJECT_NAME}_node
        ${catkin_LIBRARIES} ${PROJECT_NAME}_lib)

if(CATKIN_ENABLE_TESTING)
  catkin_add_gtest(eden_safety_test
    test/eden_safety_test.cpp)
  if(TARGET eden_safety_test)
    target_link_libraries(eden_safety_test
      ${catkin_LIBRARIES}
      gtest_main)
  endif()
endif()

install(DIRECTORY resource
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

cs_install() 
cs_export()
