add_library(CANOpenIO SHARED CANOpenIO.cpp)
target_link_libraries(CANOpenIO PUBLIC PYCH::core PYCH::core_reqs)
pych_register_plugin(CANOpenIO)