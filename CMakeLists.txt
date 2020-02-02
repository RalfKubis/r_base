set(
        target
        r_base
    )


################################################################################
#   external
################################################################################

r_qt_find_packages(
        Qt5Core
    )


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
        Qt5::Core
    )

if ( CMAKE_HOST_UNIX )

    target_link_libraries(
            ${target}
        PRIVATE
            uuid
        )

endif()
