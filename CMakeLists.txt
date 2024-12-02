x_target(r_base)


################################################################################
#   sources
################################################################################

x_add_sources(.)
x_add_sources(commandline)


################################################################################
#   target
################################################################################

add_library(
        ${target}
    STATIC
        ${sources}
    )


#   target-specific properties
x_target_add_defaults(
        ${target}
    )

#   dependencies
target_link_libraries(
        ${target}
    PUBLIC
        CONAN_PKG::stduuid
        CONAN_PKG::fmt
    PRIVATE
        CONAN_PKG::nlohmann_json
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
