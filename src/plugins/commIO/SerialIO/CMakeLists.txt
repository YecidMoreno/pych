add_library(SerialIO SHARED SerialIO.cpp)
target_link_libraries(SerialIO PUBLIC PYCH::core PYCH::core_reqs)
pych_register_plugin(SerialIO)