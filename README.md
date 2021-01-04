# OpenXR Provider Library

Provides application and engine developers a simpler method to access OpenXR runtimes (e.g. SteamVR, Oculus, WMR, Monado, etc) without having to wade through the intricacies of directly using the raw OpenXR Loader library. 

A general overview and introduction to this library can be found here: 
https://medium.com/p/6824989613b9

OpenXR Provider Library developer documentation (generated via Doxygen) can be found here:
https://github.com/1runeberg/OpenXRProvider/OpenXRProvider/docs/html/index.html

Sandbox developer documentation (generated via Doxygen) that can be used to quickly test the library and underlying api calls can be found here:
https://github.com/1runeberg/OpenXRProvider/Sandbox/docs/html/index.html


**I. Pre-built binaries**
You can find pre-built library files (.lib, .dll) as well as a Sandbox application (.exe) file for quick testing in the `bin` directory

As a quick OpenXR lifecycle demo, the Sandbox app renders a skyblue clear color to the swapchain images which then gets rendered to the hmd by the active OpenXR runtime. Sandbox also blits (copies) this texture to the desktop window (XR Mirror). 

Detailed logs in the console and `bin/logs` directory could be of particular interest to developers looking to see the OpenXR api calls being made by the library


**II. Building Pre-requisites**
Visual Studio 2019 with C++ modules
CMake 3.14.4 and above


**III. Building**
 1. Clone or copy the repository from GitHub
 2. From the root directory of the repository, create a build directory (`mkdir build`)
 3. Go to the build directory (`cd build`)
 4. Run CMake (`cmake ..`) A *openxr_provider.sln* file is generated in the build directory which you can then open and use in Visual Studio 2019
 5. Optional: Build the library and Sandbox application in CMake (`cmake --build .`)

Note: Currently supports Windows x64, OpenGL 3.2 and above
