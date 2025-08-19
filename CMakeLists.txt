cmake_minimum_required(VERSION 3.16)
project(pych_core)

set(CMAKE_CXX_STANDARD 17)

if(DEFINED TARGET_MACHINE)
    string(TOLOWER "${TARGET_MACHINE}" TARGET_MACHINE)
else()
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpmachine
        OUTPUT_VARIABLE COMPILER_TRIPLE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(TOLOWER "${COMPILER_TRIPLE}" TARGET_MACHINE)
endif()

if(DEFINED ENV{HH_CORE_WORK})
    set(CORE_PATH "$ENV{HH_CORE_WORK}")
elseif(EXISTS "/core")
    set(CORE_PATH "/core")
else()
    set(CORE_PATH ".")
endif()

message(STATUS "---\nUsing CORE_PATH as: ${CORE_PATH}\n---\n")

set(PYCH_RELATIVE_INCLUDE_DIRS
    .
    core
    utils
    src/plugins
    src/plugins_base
    libs/exprtk
    libs/cxxopts/include
    libs/rapidjson/include
)

set(PYCH_BUILD_INCLUDES_ABS "")
foreach(inc ${PYCH_RELATIVE_INCLUDE_DIRS})
    list(APPEND PYCH_BUILD_INCLUDES_ABS "${CMAKE_CURRENT_SOURCE_DIR}/${inc}")
endforeach()

set(PYCH_BUILD_INCLUDES "")
foreach(inc ${PYCH_BUILD_INCLUDES_ABS})
    list(APPEND PYCH_BUILD_INCLUDES $<BUILD_INTERFACE:${inc}>)
endforeach()

set(PYCH_INSTALL_INCLUDES "")
foreach(inc ${PYCH_RELATIVE_INCLUDE_DIRS})
    list(APPEND PYCH_INSTALL_INCLUDES $<INSTALL_INTERFACE:include/${inc}>)
endforeach()

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(PYCH_INSTALLED_TARGETS)
message("PYCH_INSTALL_INCLUDES: ${PYCH_INSTALL_INCLUDES}")

# set(RELEASE_DIR "${CMAKE_SOURCE_DIR}/__release/${TARGET_MACHINE}")
# set(LIB_DIR "${RELEASE_DIR}/lib")
# set(TEST_DIR "${RELEASE_DIR}/test")
# set(PLUGIN_DIR "${RELEASE_DIR}/plugins")

# file(MAKE_DIRECTORY "${RELEASE_DIR}")
# file(MAKE_DIRECTORY "${LIB_DIR}")
# file(MAKE_DIRECTORY "${PLUGIN_DIR}")

set(PYCH_PLUGINS_TARGET)
include(cmake/plugins.cmake)

add_subdirectory("core")

add_subdirectory("${CORE_PATH}/src/plugins/commIO/CANOpenIO")
add_subdirectory("${CORE_PATH}/src/plugins/commIO/SerialIO")

add_subdirectory("${CORE_PATH}/src/plugins/deviceIO/EPOS")
add_subdirectory("${CORE_PATH}/src/plugins/deviceIO/SensorIO")
add_subdirectory("${CORE_PATH}/src/plugins/deviceIO/ActuatorIO")

add_subdirectory("${CORE_PATH}/src/plugins/controlIO/SimplePD")
add_subdirectory("${CORE_PATH}/src/plugins/controlIO/SimplePD_1D")

add_subdirectory("${CORE_PATH}/test")
message( STATUS PYCH_PLUGINS_TARGET: ${PYCH_PLUGINS_TARGET})


foreach(target ${PYCH_PLUGINS_TARGET})
    # set_target_properties(${target} 
        # PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_DIR}  PREFIX "") 
endforeach()


include(CMakePackageConfigHelpers)

install(TARGETS core reqs 
    EXPORT pych_coreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

list(APPEND PYCH_INSTALLED_TARGETS EPOS SensorIO SerialIO testCore SimplePD SimplePD_1D)
install(TARGETS ${PYCH_INSTALLED_TARGETS} 
    EXPORT pych_coreTargets
    LIBRARY DESTINATION plugins
    ARCHIVE DESTINATION lib
)

foreach(INC ${PYCH_RELATIVE_INCLUDE_DIRS})
    if(EXISTS ${INC})
        get_filename_component(NAME ${INC} NAME)
        install(DIRECTORY ${INC}/ DESTINATION include/${INC}
            FILES_MATCHING PATTERN "*.h"
            PATTERN ".*" EXCLUDE
            PATTERN "*~" EXCLUDE
            PATTERN "*.bak" EXCLUDE
        )
    endif()
endforeach()

install(EXPORT pych_coreTargets
    FILE pych_coreTargets.cmake
    NAMESPACE pych_core::
    DESTINATION lib/cmake/pych_core
)

# Generar archivo de versión
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/pych_coreConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)
# Generar archivo de configuración
configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/pych_coreConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/pych_coreConfig.cmake"
    INSTALL_DESTINATION lib/cmake/pych_core
)

# Instalar los archivos de configuración
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/pych_coreConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/pych_coreConfigVersion.cmake"
    DESTINATION lib/cmake/pych_core
)


