add_library(ActuatorIO SHARED ActuatorIO.cpp)
target_link_libraries(ActuatorIO PUBLIC PYCH::core PYCH::core_reqs)
pych_register_plugin(ActuatorIO)