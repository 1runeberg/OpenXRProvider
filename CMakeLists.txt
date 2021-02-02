cmake_minimum_required(VERSION 3.14.4)
set(CMAKE_SUPPRESS_REGENERATION true)
project(openxr_provider)

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

# Set the sandbox as the startup project in VS
set_property (DIRECTORY PROPERTY VS_STARTUP_PROJECT "Sandbox")

# OpenXR Provider
add_subdirectory(OpenXRProvider)

# Add additional libraries
set (LIBS_INCLUDE 
    ${CMAKE_SOURCE_DIR}/OpenXRProvider/include
    ${CMAKE_SOURCE_DIR}/OpenXRProvider/spdlog
    ${CMAKE_SOURCE_DIR}/Sandbox/third_party
    ${CMAKE_SOURCE_DIR}/Sandbox/third_party/glad/include
    )

set (LIBS_SOURCES
    ${CMAKE_SOURCE_DIR}/Sandbox/third_party/glad/src/glad.c)

# Add project public include directories
set (OPENXR_SANDBOX_INCLUDE
        ${CMAKE_SOURCE_DIR}/Sandbox
	${CMAKE_SOURCE_DIR}/Sandbox/include
        ${LIBS_INCLUDE}
	)
message(STATUS "Added OpenXR Sandbox public include directories: 
    \ ${OPENXR_SANDBOX_INCLUDE}")

# Find all project header files
file(GLOB OPENXR_SANDBOX_HEADERS
        "${CMAKE_SOURCE_DIR}/Sandbox/*.h" 
	"${CMAKE_SOURCE_DIR}/Sandbox/include/*.h" 
        "${CMAKE_SOURCE_DIR}/Sandbox/src/*.h"
	)
message(STATUS "Found OpenXR Sandbox headers
    \ ${OPENXR_SANDBOX_HEADERS}")


# Find all project source files
file(GLOB OPENXR_SANDBOX_SOURCE
        "${CMAKE_SOURCE_DIR}/Sandbox/*.cpp"
	"${CMAKE_SOURCE_DIR}/Sandbox/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/Sandbox/src/shaders/*.glsl"
        "${CMAKE_SOURCE_DIR}/Sandbox/third_party/stb/*.cpp"
	)
message(STATUS "Found OpenXR Sandbox source
    \  ${OPENXR_SANDBOX_SOURCE}")

# Set project public include directories
include_directories(Sandbox PUBLIC
    ${OPENXR_SANDBOX_INCLUDE}
    )

# Add project header and source files to project
add_executable(Sandbox 
    ${OPENXR_SANDBOX_HEADERS} 
    ${OPENXR_SANDBOX_SOURCE}
    ${LIBS_SOURCES})

# Set output directories
set_target_properties(Sandbox PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)

# SDL2
set(LIB_SDL2 
	"${CMAKE_SOURCE_DIR}/Sandbox/third_party/sdl/lib/x64/SDL2.lib"
	"${CMAKE_SOURCE_DIR}/Sandbox/third_party/sdl/lib/x64/SDL2main.lib"
	)

# OpenGL
set(LIB_OPENGL "opengl32.lib")

# Add libraries
target_link_libraries(Sandbox PUBLIC 
    ${LIB_SDL2}
    opengl32.lib
    OpenXRProvider)

# Post-Build
add_custom_command(TARGET Sandbox POST_BUILD

	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin/logs

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/third_party/sdl/lib/x64/SDL2.lib
            ${CMAKE_SOURCE_DIR}/bin/SDL2.lib

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/third_party/sdl/lib/x64/SDL2.dll
            ${CMAKE_SOURCE_DIR}/bin/SDL2.dll

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/OpenXRProvider/third_party/openxr_loader_windows/x64/lib/openxr_loader.lib
            ${CMAKE_SOURCE_DIR}/bin/openxr_loader.lib

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/OpenXRProvider/third_party/openxr_loader_windows/x64/bin/openxr_loader.dll
            ${CMAKE_SOURCE_DIR}/bin/openxr_loader.dll
	
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/src/shaders/vert-vismask.glsl
            ${CMAKE_SOURCE_DIR}/bin/shaders/vert-vismask.glsl

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/src/shaders/frag-vismask.glsl
            ${CMAKE_SOURCE_DIR}/bin/shaders/frag-vismask.glsl

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/src/shaders/vert-lit.glsl
            ${CMAKE_SOURCE_DIR}/bin/shaders/vert-lit.glsl

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/src/shaders/frag-lit.glsl
            ${CMAKE_SOURCE_DIR}/bin/shaders/frag-lit.glsl
         
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/src/shaders/vert-unlit.glsl
            ${CMAKE_SOURCE_DIR}/bin/shaders/vert-unlit.glsl

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/src/shaders/frag-unlit.glsl
            ${CMAKE_SOURCE_DIR}/bin/shaders/frag-unlit.glsl

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/src/shaders/vert-textured.glsl
            ${CMAKE_SOURCE_DIR}/bin/shaders/vert-textured.glsl

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/src/shaders/frag-textured.glsl
            ${CMAKE_SOURCE_DIR}/bin/shaders/frag-textured.glsl

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/img/t_bellevue_valve.png
            ${CMAKE_SOURCE_DIR}/bin/img/t_bellevue_valve.png

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/img/t_hobart_mein_heim.png
            ${CMAKE_SOURCE_DIR}/bin/img/t_hobart_mein_heim.png
            
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/img/t_hobart_mein_kochen.png
            ${CMAKE_SOURCE_DIR}/bin/img/t_hobart_mein_kochen.png

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/img/t_hobart_rose.png
            ${CMAKE_SOURCE_DIR}/bin/img/t_hobart_rose.png

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/img/t_hobart_sunset.png
            ${CMAKE_SOURCE_DIR}/bin/img/t_hobart_sunset.png
            
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/img/t_munich_mein_schatz.png
            ${CMAKE_SOURCE_DIR}/bin/img/t_munich_mein_schatz.png

        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/Sandbox/img/t_munich_mein_schatz.png
            ${CMAKE_SOURCE_DIR}/bin/img/t_munich_mein_schatz.png

		)
