r_target(r_base)

################################################################################
#   external
################################################################################


################################################################################
#   sources
################################################################################

add_sources(.)
add_sources(commandline)


################################################################################
#   target
################################################################################

add_library(
        ${target}
    STATIC
        ${sources}
    )


#   target-specific properties
r_target_add_defaults(
        ${target}
    )

#   dependencies
target_link_libraries(
        ${target}
    PUBLIC
        ${META_PROJECT_NAME}::fmt
    )

if ( CMAKE_HOST_UNIX )

    target_link_libraries(
            ${target}
        PRIVATE
            uuid
        )

endif()


if ( CMAKE_HOST_WIN32 )

    target_link_libraries(
            ${target}
        PRIVATE
            Psapi
        )

endif()
