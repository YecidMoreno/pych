add_library(SensorIO SHARED SensorIO.cpp)
target_include_directories(SensorIO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SensorIO PUBLIC reqs)
set_target_properties(SensorIO PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_DEVICEIO_DIR}
    PREFIX ""
)
add_library(deviceIO::SensorIO ALIAS SensorIO)

