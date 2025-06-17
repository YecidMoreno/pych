# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${RELEASE_DIR}/lib")


message("Core includes: ${build_includes}")

add_library(core SHARED core.cpp tasking.cpp)
target_include_directories(core
    PUBLIC
    ${PYCH_BUILD_INCLUDES}
    ${PYCH_INSTALL_INCLUDES}
)


add_library(reqs SHARED json_api.cpp exprtk_wrapper.cpp record_variables.cpp)
target_include_directories(reqs
    PUBLIC
    ${PYCH_BUILD_INCLUDES}
    $<INSTALL_INTERFACE:include>
)


target_link_libraries(core PUBLIC reqs)
add_library(HH::core ALIAS core)

set_target_properties(core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

# list(APPEND PYCH_INSTALLED_TARGETS core reqs)
