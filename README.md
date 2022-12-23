# OpenXR Provider Library

## NOTE: This is now DEPRECATED - new version up: https://github.com/1runeberg/OpenXRProvider_v2


NOTE: Sandbox test app requires SteamVR Beta 1.16


A general overview and introduction to this library can be found here: 


https://runeberg.medium.com/the-dawn-of-openxr-6824989613b9


OpenXR Provider Library developer documentation (generated via Doxygen) can be found in:


http://runeberg.io/OpenXRProvider/html/


Sandbox developer documentation (generated via Doxygen) that can be used to quickly test the library and underlying api calls can be found here:

`Sandbox\docs`


**I. Pre-built binaries**

You can find pre-built library files (.lib, .dll) as well as a OpenGL Sandbox application (.exe)  for quick testing in the `bin` directory. This requires SteamVR Beta 1.16

As a quick OpenXR lifecycle demo, the Sandbox app renders a skyblue clear color to the swapchain images which then gets rendered to the hmd by the active OpenXR runtime. Sandbox also blits (copies) this texture to the desktop window (XR Mirror). 

Detailed logs in the console and `bin/logs` directory could be of particular interest to developers looking to see the OpenXR api calls being made by the library


**II. Building Pre-requisites**

Visual Studio 2019 with C++ modules (or VS 2017)
CMake 3.14.4 and above (https://cmake.org/download/) - Install with include in system path option


**III. Building**

 1. Clone or copy the repository from GitHub
 2. From the root directory of the repository, create a build directory (`mkdir build`)
 3. Go to the build directory (`cd build`)
 4. Run CMake (`cmake ..` or for VS 2017 `cmake -G "Visual Studio 15 2017 Win64" ..`) A *openxr_provider.sln* file is generated in the build directory which you can then open and use in Visual Studio 2019
 5. Optional: Build the library and Sandbox application in CMake (`cmake --build .`)

Note: Currently supports Windows x64, OpenGL 3.2 and above
