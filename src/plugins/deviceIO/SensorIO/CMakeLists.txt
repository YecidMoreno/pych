add_library(SensorIO SHARED SensorIO.cpp)
target_link_libraries(SensorIO PUBLIC PYCH::core PYCH::core_reqs)
pych_register_plugin(SensorIO)