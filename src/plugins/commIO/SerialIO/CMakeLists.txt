
add_library(SerialIO SHARED SerialIO.cpp)
target_include_directories(SerialIO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SerialIO PUBLIC reqs)
set_target_properties(SerialIO PROPERTIES 
LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_COMMIO_DIR}
PREFIX "")

add_library(commIO::SerialIO ALIAS SerialIO)