add_library(SensorIO SHARED SensorIO.cpp)

target_link_libraries(SensorIO PUBLIC reqs core)
target_include_directories(SensorIO
    PUBLIC
    ${PYCH_BUILD_INCLUDES}
    ${PYCH_INSTALL_INCLUDES}
)
add_library(deviceIO::SensorIO ALIAS SensorIO)

set_target_properties(SensorIO PROPERTIES 
LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_DEVICEIO_DIR}
PREFIX "")