add_library(CANOpenIO SHARED CANOpenIO.cpp)
target_include_directories(CANOpenIO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CANOpenIO PUBLIC reqs)
target_link_libraries(CANOpenIO PUBLIC HH::core)
set_target_properties(CANOpenIO PROPERTIES 
LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_COMMIO_DIR}
PREFIX "")

add_library(commIO::CANOpen ALIAS CANOpenIO)