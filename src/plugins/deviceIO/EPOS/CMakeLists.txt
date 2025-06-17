add_library(EPOS SHARED  EPOS.cpp)

target_link_libraries(EPOS PUBLIC)
target_include_directories(EPOS
    PUBLIC
    ${PYCH_BUILD_INCLUDES}
    $<INSTALL_INTERFACE:include>
)
set_target_properties(EPOS PROPERTIES 
LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_DEVICEIO_DIR}
PREFIX "")


add_library(deviceIO::EPOS ALIAS EPOS)
# list(APPEND PYCH_INSTALLED_TARGETS EPOS)