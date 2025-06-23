add_library(SimplePD SHARED SimplePD.cpp)
target_link_libraries(SimplePD PUBLIC PYCH::core PYCH::core_reqs)
pych_register_plugin(SimplePD)