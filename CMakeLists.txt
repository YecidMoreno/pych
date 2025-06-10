cmake_minimum_required(VERSION 3.16)
project(signal_io_main)

set(CMAKE_CXX_STANDARD 17)

include_directories(".")
include_directories("src/plugins_base")
include_directories("utils")
include_directories("src/plugins")
include_directories("libs/exprtk")
include_directories("libs/cxxopts/include")
include_directories("libs/rapidjson/include")

if(DEFINED TARGET_PWD)
    string(TOLOWER "${TARGET_PWD}" ARCH)
else()
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCH)
endif()


set(RELEASE_DIR "${CMAKE_SOURCE_DIR}/release/${ARCH}")
set(TEST_DIR "${RELEASE_DIR}/test")
set(PLUGIN_DIR "${RELEASE_DIR}/plugins")
set(PLUGIN_COMMIO_DIR "${RELEASE_DIR}/plugins/commIO")
set(PLUGIN_DEVICEIO_DIR "${RELEASE_DIR}/plugins/deviceIO")
set(PLUGIN_CONTROLIO_DIR "${RELEASE_DIR}/plugins/controlIO")


file(MAKE_DIRECTORY "${RELEASE_DIR}")
file(MAKE_DIRECTORY "${PLUGIN_DIR}")
file(MAKE_DIRECTORY "${PLUGIN_COMMIO_DIR}")
file(MAKE_DIRECTORY "${PLUGIN_DEVICEIO_DIR}")
file(MAKE_DIRECTORY "${PLUGIN_CONTROLIO_DIR}")


add_subdirectory("core")

add_subdirectory("src/plugins/commIO/CANOpenIO")
add_subdirectory("src/plugins/commIO/SerialIO")
add_subdirectory("src/plugins/deviceIO/EPOS")
add_subdirectory("src/plugins/deviceIO/SensorIO")
add_subdirectory("src/plugins/deviceIO/ActuatorIO")

add_subdirectory("src/plugins/controlIO/SimplePD")

add_subdirectory("test")

