# FindBinaryNinja.cmake - Find Binary Ninja API when building inside Binary Ninja

if(NOT BN_API_PATH)
    message(FATAL_ERROR "BN_API_PATH must be set")
endif()

if(NOT BN_INSTALL_DIR)
    message(FATAL_ERROR "BN_INSTALL_DIR must be set")
endif()

# Set up binaryninjaapi library
if(NOT TARGET binaryninjaapi)
    add_library(binaryninjaapi STATIC IMPORTED GLOBAL)
    if(WIN32)
        set_target_properties(binaryninjaapi PROPERTIES
            IMPORTED_LOCATION "${BN_API_PATH}/../build/api/out/binaryninjaapi.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${BN_API_PATH};${BN_API_PATH}/vendor/fmt/include"
        )
    else()
        set_target_properties(binaryninjaapi PROPERTIES
            IMPORTED_LOCATION "${BN_API_PATH}/../build/api/out/libbinaryninjaapi.a"
            INTERFACE_INCLUDE_DIRECTORIES "${BN_API_PATH};${BN_API_PATH}/vendor/fmt/include"
        )
    endif()
endif()

# Set up binaryninjacore library (use the actual one, not stubs)
if(NOT TARGET binaryninjacore)
    add_library(binaryninjacore SHARED IMPORTED GLOBAL)
    if(APPLE)
        set_target_properties(binaryninjacore PROPERTIES
            IMPORTED_LOCATION "${BN_INSTALL_DIR}/libbinaryninjacore.dylib"
            IMPORTED_SONAME "@rpath/libbinaryninjacore.1.dylib"
            INTERFACE_INCLUDE_DIRECTORIES "${BN_API_PATH}"
        )
    elseif(UNIX)
        set_target_properties(binaryninjacore PROPERTIES
            IMPORTED_LOCATION "${BN_INSTALL_DIR}/libbinaryninjacore.so"
            IMPORTED_SONAME "libbinaryninjacore.so.1"
            INTERFACE_INCLUDE_DIRECTORIES "${BN_API_PATH}"
        )
    elseif(WIN32)
        # For Windows, we need to set configuration-agnostic properties
        set_target_properties(binaryninjacore PROPERTIES
            IMPORTED_LOCATION_RELEASE "${BN_INSTALL_DIR}/binaryninjacore.dll"
            IMPORTED_IMPLIB_RELEASE "${BN_INSTALL_DIR}/binaryninjacore.lib"
            IMPORTED_LOCATION_DEBUG "${BN_INSTALL_DIR}/binaryninjacore.dll"
            IMPORTED_IMPLIB_DEBUG "${BN_INSTALL_DIR}/binaryninjacore.lib"
            IMPORTED_LOCATION_RELWITHDEBINFO "${BN_INSTALL_DIR}/binaryninjacore.dll"
            IMPORTED_IMPLIB_RELWITHDEBINFO "${BN_INSTALL_DIR}/binaryninjacore.lib"
            IMPORTED_LOCATION_MINSIZEREL "${BN_INSTALL_DIR}/binaryninjacore.dll"
            IMPORTED_IMPLIB_MINSIZEREL "${BN_INSTALL_DIR}/binaryninjacore.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${BN_API_PATH}"
        )
    endif()
endif()

# Set up fmt library (needed by Binary Ninja API)
if(NOT TARGET fmt::fmt)
    add_library(fmt STATIC IMPORTED GLOBAL)
    if(WIN32)
        set_target_properties(fmt PROPERTIES
            IMPORTED_LOCATION "${BN_API_PATH}/../build/api/vendor/fmt/fmt.lib"
        )
    else()
        set_target_properties(fmt PROPERTIES
            IMPORTED_LOCATION "${BN_API_PATH}/../build/api/vendor/fmt/libfmt.a"
        )
    endif()
    add_library(fmt::fmt ALIAS fmt)
endif()

# Make binaryninjaapi depend on fmt and binaryninjacore
set_target_properties(binaryninjaapi PROPERTIES
    INTERFACE_LINK_LIBRARIES "fmt::fmt;binaryninjacore"
)

# Create alias
if(NOT TARGET BinaryNinja::API)
    add_library(BinaryNinja::API ALIAS binaryninjaapi)
endif()

set(BinaryNinja_FOUND TRUE)
set(binaryninjaapi_SOURCE_DIR "${BN_API_PATH}")