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
