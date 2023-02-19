# Find PIPEWIRE v0.3 (w/ SPA 0.2)
#
# @var PIPEWIRE_FOUND        PIPEWIRE found flag
# @var PIPEWIRE_LIBRARIES    location of the library
# @var PIPEWIRE_INCLUDE_DIRS location of the include files
# @var PIPEWIRE_VERSION      version


find_package(PkgConfig QUIET)

pkg_search_module(PKG_PipeWire QUIET libpipewire-0.3)
pkg_search_module(PKG_Spa QUIET libspa-0.2)

set(PipeWire_DEFINITIONS "${PKG_PipeWire_CFLAGS}" "${PKG_Spa_CFLAGS}")
set(PipeWire_VERSION "${PKG_PipeWire_VERSION}")


find_path(PIPEWIRE_INCLUDE_DIRS
        NAMES pipewire/pipewire.h
        HINTS ${PKG_PipeWire_INCLUDE_DIRS}
              ${PKG_PipeWire_INCLUDE_DIRS}/pipewire-0.3
)

find_path(SPA_INCLUDE_DIRS
        NAMES spa/param/props.h
        HINTS ${PKG_Spa_INCLUDE_DIRS}
              ${PKG_Spa_INCLUDE_DIRS}/spa-0.2
)

find_library(PIPEWIRE_LIBRARIES
        NAMES pipewire-0.3
        HINTS ${PKG_PipeWire_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(PipeWire
        FOUND_VAR     PIPEWIRE_FOUND
        REQUIRED_VARS PIPEWIRE_LIBRARIES PIPEWIRE_INCLUDE_DIRS SPA_INCLUDE_DIRS
        VERSION_VAR   PIPEWIRE_VERSION
)

if(PIPEWIRE_FOUND AND NOT TARGET PipeWire::PipeWire)
    add_library(PipeWire::PipeWire UNKNOWN IMPORTED)

    set_target_properties(PipeWire::PipeWire
            PROPERTIES IMPORTED_LOCATION             "${PIPEWIRE_LIBRARIES}"
                       INTERFACE_COMPILE_OPTIONS     "${PIPEWIRE_DEFINITIONS}"
                       INTERFACE_INCLUDE_DIRECTORIES "${PIPEWIRE_INCLUDE_DIRS};${SPA_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(PIPEWIRE_LIBRARIES PIPEWIRE_INCLUDE_DIRS)

include(FeatureSummary)

set_package_properties(PipeWire PROPERTIES
        URL "https://www.pipewire.org"
        DESCRIPTION "PipeWire"
)