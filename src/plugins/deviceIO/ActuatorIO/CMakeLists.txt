add_library(ActuatorIO SHARED ActuatorIO.cpp)
target_include_directories(ActuatorIO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ActuatorIO PUBLIC reqs)
set_target_properties(ActuatorIO PROPERTIES 
LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_DEVICEIO_DIR}
PREFIX "")

add_library(deviceIO::ActuatorIO ALIAS ActuatorIO)