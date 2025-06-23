add_library(EPOS SHARED EPOS.cpp)
target_link_libraries(EPOS PUBLIC PYCH::core PYCH::core_reqs)
pych_register_plugin(EPOS)