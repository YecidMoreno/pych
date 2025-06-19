add_library(SimplePD SHARED SimplePD.cpp)

target_link_libraries(SimplePD PUBLIC reqs core)
target_include_directories(SimplePD
    PUBLIC
    ${PYCH_BUILD_INCLUDES}
    ${PYCH_INSTALL_INCLUDES}
)
add_library(deviceIO::SimplePD ALIAS SimplePD)

set_target_properties(SimplePD PROPERTIES 
LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_CONTROLIO_DIR}
PREFIX "")