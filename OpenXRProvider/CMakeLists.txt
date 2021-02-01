cmake_minimum_required(VERSION 3.14.4)
set(CMAKE_SUPPRESS_REGENERATION true)
project(OpenXRProvider)

# C++ 17 - note cmake minver is 3.10 supported only on MS C++ 2015 Update 3 and above
message(STATUS "Project language set to C++17")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Check platform architecture
if(NOT PLATFORM)
	if(CMAKE_SIZEOF_VOID_P MATCHES 8)
	    set(PLATFORM 64)
	else()
        message(FATAL_ERROR "ERROR: Only 32-bit platforms are supported.")
	endif()
endif()

# Check OS
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_definitions(-D_WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS True)
endif()

# Header only libs (spdlog)
set (INCLUDE_HEADER_LIBS ${CMAKE_SOURCE_DIR}/OpenXRProvider/third_party)

# Set OpenXR location
set (OPENXR_DIR ${CMAKE_SOURCE_DIR}/OpenXRProvider/third_party/openxr_loader_windows)
set (INCLUDE_OPENXR ${OPENXR_DIR}/include)

set (LIB_OPENXR_FILE openxr_loader.lib)
set (BIN_OPENXR_FILE openxr_loader.dll)

set (LIB_OPENXR "${OPENXR_DIR}/x64/lib/${LIB_OPENXR_FILE}")
set (BIN_OPENXR "${OPENXR_DIR}/x64/bin/${BIN_OPENXR_FILE}")

# OpenXR Provider
file(GLOB OPENXR_PROVIDER_INCLUDE
	"${CMAKE_SOURCE_DIR}/OpenXRProvider/*.h"
    "${CMAKE_SOURCE_DIR}/OpenXRProvider/include/*.h"
    "${CMAKE_SOURCE_DIR}/OpenXRProvider/include/extensions/*.h"
    "${CMAKE_SOURCE_DIR}/OpenXRProvider/include/rendering/*.h"
    "${CMAKE_SOURCE_DIR}/OpenXRProvider/include/input/*.h"
    "${CMAKE_SOURCE_DIR}/OpenXRProvider/src/*.h"
	)

file(GLOB OPENXR_PROVIDER_SOURCE
	"${CMAKE_SOURCE_DIR}/OpenXRProvider/*.cpp"
    "${CMAKE_SOURCE_DIR}/OpenXRProvider/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/OpenXRProvider/src/extensions/*.cpp"
    "${CMAKE_SOURCE_DIR}/OpenXRProvider/src/rendering/*.cpp"
    "${CMAKE_SOURCE_DIR}/OpenXRProvider/src/input/*.cpp"
	)

add_library(OpenXRProvider SHARED ${OPENXR_PROVIDER_INCLUDE} ${OPENXR_PROVIDER_SOURCE})
target_include_directories(OpenXRProvider PUBLIC
    ${INCLUDE_HEADER_LIBS}
    ${INCLUDE_OPENXR}
    ${CMAKE_SOURCE_DIR}/OpenXRProvider
    ${CMAKE_SOURCE_DIR}/OpenXRProvider/include)

include(GenerateExportHeader)
generate_export_header(OpenXRProvider)

# Set output directories
set_target_properties(OpenXRProvider PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)


# OpenXR
add_library(OpenXR SHARED IMPORTED) 
set_target_properties(OpenXR PROPERTIES IMPORTED_IMPLIB ${LIB_OPENXR})
set_target_properties(OpenXR PROPERTIES IMPORTED_LOCATION ${BIN_OPENXR})
set_target_properties(OpenXR PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(OpenXRProvider PUBLIC OpenXR)
message(STATUS "OPENXR library added: ${LIB_OPENXR}")
