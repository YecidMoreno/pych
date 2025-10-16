# function(register_plugin target)
#     list(APPEND PYCH_PLUGINS_TARGET ${target})
#     set(PYCH_PLUGINS_TARGET ${PYCH_PLUGINS_TARGET} PARENT_SCOPE)
# endfunction()

function(pych_register_plugin target)
    target_include_directories(${target}
        PUBLIC
        ${PYCH_BUILD_INCLUDES}
        ${PYCH_INSTALL_INCLUDES}
    )
    list(APPEND PYCH_PLUGINS_TARGET ${target})
    list(REMOVE_DUPLICATES PYCH_PLUGINS_TARGET)
    set(PYCH_PLUGINS_TARGET "${PYCH_PLUGINS_TARGET}" CACHE INTERNAL "")

endfunction()

function(pych_register_extern_plugin LIB_NAME LIB_PATH)

    set(LIB_PATH ".")
    if(ARGC GREATER 1)
        set(LIB_PATH ${ARGV1})
    endif()

    message(STATUS "[PYCH]  Registering external plugin: ${LIB_NAME}")
    add_library(${LIB_NAME} SHARED ${LIB_PATH}/${LIB_NAME}.cpp)
    target_link_libraries(${LIB_NAME} PUBLIC pych_core::core pych_core::reqs)

    list(APPEND PYCH_INSTALLED_TARGETS ${LIB_NAME})
    set(PYCH_INSTALLED_TARGETS "${PYCH_INSTALLED_TARGETS}" CACHE INTERNAL "List of installed targets")
endfunction()
