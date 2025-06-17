add_library(SensorIO SHARED SensorIO.cpp)

target_link_libraries(SensorIO PUBLIC reqs)
target_include_directories(SensorIO
    PUBLIC
    ${PYCH_BUILD_INCLUDES}
    $<INSTALL_INTERFACE:include>
)
add_library(deviceIO::SensorIO ALIAS SensorIO)

