add_library(EPOS SHARED  EPOS.cpp)

target_include_directories(EPOS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(EPOS PUBLIC)
set_target_properties(EPOS PROPERTIES 
LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_DEVICEIO_DIR}
PREFIX "")

add_library(deviceIO::EPOS ALIAS EPOS)