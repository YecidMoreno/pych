set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${RELEASE_DIR}/lib")
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${RELEASE_DIR}/lib")

add_library(reqs SHARED json_api.cpp exprtk_wrapper.cpp)

add_library(core SHARED core.cpp tasking.cpp)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core PUBLIC reqs)
add_library(HH::core ALIAS core)

set_target_properties(core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)