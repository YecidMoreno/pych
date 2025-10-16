cmake_minimum_required(VERSION 3.16)
project(plugin_build)
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

set(RELEASE_DIR "${CMAKE_SOURCE_DIR}/release/${TARGET_MACHINE}")
file(MAKE_DIRECTORY "${RELEASE_DIR}")


find_package(pych_core REQUIRED PATHS /core /core/release/${TARGET_MACHINE})

get_property(DEFINED_FUNCTIONS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILTIN_FUNCTIONS)
message(STATUS "Defined functions: ${DEFINED_FUNCTIONS}")
get_property(DEFINED_MACROS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY MACROS)
message(STATUS "Defined macros: ${DEFINED_MACROS}")

set(CMAKE_SHARED_LIBRARY_PREFIX "")

unset(PYCH_INSTALLED_TARGETS CACHE)
set(PYCH_INSTALLED_TARGETS "" CACHE INTERNAL "List of installed targets")

include_directories(".")

file(GLOB SUBDIRS RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/*)
foreach(SUBDIR ${SUBDIRS})
    if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${SUBDIR})
        # Exclude directories starting with "release", ".", or "~"
        if(NOT SUBDIR MATCHES "^(build|cmake|release|\\.|~).*")
            message(STATUS "Adding subdirectory: ${SUBDIR}")
            
            #if CmakeList exists in the subdirectory, add it
            if(EXISTS ${CMAKE_SOURCE_DIR}/${SUBDIR}/CMakeLists.txt)
                message(STATUS "Found CMakeLists.txt in ${SUBDIR}, adding subdirectory.")
            else()
                message(STATUS "No CMakeLists.txt found in ${SUBDIR}, skipping.")
                
                # add all *.cppfiles in the subdirectory as a library
                file(GLOB CPP_FILES RELATIVE ${CMAKE_SOURCE_DIR}/${SUBDIR} ${CMAKE_SOURCE_DIR}/${SUBDIR}/*.cpp)
                if(CPP_FILES)
                    foreach(CPP_FILE ${CPP_FILES})
                        # Get the filename without extension
                        get_filename_component(TARGET_NAME ${CPP_FILE} NAME_WE)
                        message(STATUS "Adding library target: ${TARGET_NAME} from file: ${CPP_FILE}")
                        pych_register_extern_plugin(${TARGET_NAME} "${SUBDIR}")
                    endforeach()
                else()
                    message(STATUS "No .cpp files found in ${SUBDIR}, skipping library creation.")
                endif()

                continue()
            endif()
            add_subdirectory(${SUBDIR})

        endif()
    endif()
endforeach()


if(EXISTS ${CMAKE_SOURCE_DIR}/_CMakeLists.txt)
    message(STATUS "Including _CMakeLists.txt")
    include(_CMakeLists.txt)
else()
    message(STATUS "No _CMakeLists.txt found, skipping.")
endif()

install(TARGETS ${PYCH_INSTALLED_TARGETS} 
    LIBRARY DESTINATION plugins
    ARCHIVE DESTINATION plugins
)

