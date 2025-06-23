add_library(SimplePD_1D SHARED SimplePD_1D.cpp)
target_link_libraries(SimplePD_1D PUBLIC PYCH::core PYCH::core_reqs)
pych_register_plugin(SimplePD_1D)