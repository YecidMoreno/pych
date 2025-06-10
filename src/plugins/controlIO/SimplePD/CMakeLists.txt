add_library(SimplePD SHARED SimplePD.cpp)
target_include_directories(SimplePD PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SimplePD PUBLIC reqs)
set_target_properties(SimplePD PROPERTIES 
LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_CONTROLIO_DIR}
PREFIX "")

add_library(deviceIO::SimplePD ALIAS SimplePD)