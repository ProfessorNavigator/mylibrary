find_path(DJVU_INCLUDE_DIR
    NAMES ddjvuapi.h miniexp.h
    PATH_SUFFIXES "libdjvu"
)

find_library(DJVU_LIBRARY
    NAMES djvulibre
    REQUIRED
)

include(FindPackageHandleStandardArgs)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_file(DJVU_DLL
        NAMES libdjvulibre.dll libdjvulibre-21.dll
    )

    if(DJVU_INCLUDE_DIR AND DJVU_LIBRARY AND DJVU_DLL)
        add_library(djvu SHARED IMPORTED GLOBAL)
        set_target_properties(djvu PROPERTIES
            IMPORTED_LOCATION "${DJVU_DLL}"
            IMPORTED_IMPLIB "${DJVU_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${DJVU_INCLUDE_DIR}"
        )
    endif()

    find_package_handle_standard_args(DJVU
        FOUND_VAR DJVU_FOUND
        REQUIRED_VARS DJVU_INCLUDE_DIR DJVU_LIBRARY DJVU_DLL
    )
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(DJVU_INCLUDE_DIR AND DJVU_LIBRARY)
        add_library(djvu SHARED IMPORTED GLOBAL)
        set_target_properties(djvu PROPERTIES
            IMPORTED_IMPLIB "${DJVU_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${DJVU_INCLUDE_DIR}"
        )
    endif()

    find_package_handle_standard_args(DJVU
        FOUND_VAR DJVU_FOUND
        REQUIRED_VARS DJVU_INCLUDE_DIR DJVU_LIBRARY
    )
endif()
