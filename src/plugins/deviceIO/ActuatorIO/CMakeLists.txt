
add_library(ActuatorIO SHARED ActuatorIO.cpp)

target_link_libraries(ActuatorIO PUBLIC reqs core)
target_include_directories(ActuatorIO
    PUBLIC
    ${PYCH_BUILD_INCLUDES}
    ${PYCH_INSTALL_INCLUDES}
)
add_library(deviceIO::ActuatorIO ALIAS ActuatorIO)

set_target_properties(ActuatorIO PROPERTIES 
LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_DEVICEIO_DIR}
PREFIX "")