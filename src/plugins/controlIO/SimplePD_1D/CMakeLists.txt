add_library(SimplePD_1D SHARED SimplePD_1D.cpp)

target_link_libraries(SimplePD_1D PUBLIC reqs)
target_include_directories(SimplePD_1D
    PUBLIC
    ${PYCH_BUILD_INCLUDES}
    ${PYCH_INSTALL_INCLUDES}
)
add_library(deviceIO::SimplePD_1D ALIAS SimplePD_1D)

set_target_properties(SimplePD_1D PROPERTIES 
LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_CONTROLIO_DIR}
PREFIX "")