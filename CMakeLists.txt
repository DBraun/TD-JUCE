cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(TD-JUCE VERSION 0.0.1)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TD-JUCE)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")

endif()

# https://github.com/OpenShot/libopenshot-audio/blob/7b76f1de9c6a111f1aba2979eeb4a847a93aeeb4/CMakeLists.txt#L112
# Juce requires either DEBUG or NDEBUG to be defined on MacOS.
# -DNDEBUG is set by cmake for all release configs, so add
# -DDEBUG for debug builds. We'll do this for all OSes, even
# though only MacOS requires it.
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
# # Make sure we've picked some build type, default to debug
# if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
#     set(CMAKE_BUILD_TYPE "Debug")
# endif()


################################################################################
# Global linker options
################################################################################
# if(MSVC)
#     # remove default flags provided with CMake for MSVC
#     set(CMAKE_EXE_LINKER_FLAGS "")
#     set(CMAKE_MODULE_LINKER_FLAGS "")
#     set(CMAKE_SHARED_LINKER_FLAGS "")
#     set(CMAKE_STATIC_LINKER_FLAGS "")
#     set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
#     set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
#     set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
#     set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
#     set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
#     set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
#     set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
#     set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
# endif()
set(cxx_no_exception_flags "-EHs-c- -D_HAS_EXCEPTIONS=1")

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################
project(TD-JUCE VERSION 0.0.1)

set(JUCE_MODULES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/JUCE_6/modules" CACHE PATH
    "Location of the JUCE source code 'modules' directory")

message(STATUS "Using JUCE module path: ${JUCE_MODULES_PATH}")

# Default extension for source files
if(UNIX AND APPLE)
  set(SOURCE_EXTENSION "mm")
else ()
  set(SOURCE_EXTENSION "cpp")
endif()

# https://forum.juce.com/t/juce-api-is-not-set-to---declspec-dllexport-when-creating-dynamic-library-project/22962/2
add_compile_definitions(JUCE_DLL_BUILD=1)

# List of modules to build
set(JUCE_MODULES
    juce_audio_basics
    juce_audio_formats
    juce_core
    juce_data_structures
    juce_dsp
    juce_events)

# Convert to list of source files (extension based on OS)
foreach(j_module IN LISTS JUCE_MODULES)
    list(APPEND JUCE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/include_${j_module}.${SOURCE_EXTENSION} )
endforeach()

list(APPEND JUCE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/AppConfig.h")
list(APPEND JUCE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/JuceHeader.h")

add_library(TD-JUCE SHARED ${JUCE_SOURCES} )

# Include header directories
target_include_directories(TD-JUCE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode>
    $<BUILD_INTERFACE:${JUCE_MODULES_PATH}>
    $<INSTALL_INTERFACE:TD-JUCE> )

# Install library
INSTALL(TARGETS TD-JUCE
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )

set_target_properties(${PROJECT_NAME} PROPERTIES
                      VS_DEBUGGER_COMMAND "C:\\Program Files\\Derivative\\TouchDesigner\\bin\\TouchDesigner.exe"
                      VS_DEBUGGER_COMMAND_ARGUMENTS "..\\$(ProjectName).toe")

target_compile_options(TD-JUCE PRIVATE /EHsc /GR)

#The following step will create a post-build event that copies the custom DLL to
#the Documents/Derivative/Plugins folder.
if (MSVC)
  add_custom_command(TARGET ${PROJECT_NAME}
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     "$<TARGET_FILE:TD-JUCE>"
                     ${CMAKE_SOURCE_DIR}/Plugins)
endif (MSVC)

### add all projects for DLLs that can be used inside TouchDesigner
add_subdirectory(TD-JUCE)
