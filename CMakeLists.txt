cmake_minimum_required(VERSION 3.14)

# LCG sets CPATH, which gets treated like -I by the compiler. We want to ignore
# warnings from libraries, by unsetting it here, it gets processed by the usual
# target_include_directories call, resulting in the desired -isystem flag.
unset(ENV{CPATH})

# must be set before project(...) call; version module is needed before
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# determine project version; sets _acts_version and _acts_commit_hash
include(ActsRetrieveVersion)

project(Acts VERSION ${_acts_version} LANGUAGES CXX)

# build options

# all options and compile-time parameters must be defined here for clear
# visibility and to make them available everywhere
#
# NOTE if you are adding a new option make sure that is defined in such a way
#   that it is off/empty by default. if you think that is not possible, then
#   it probably is not an optional component.
option(ACTS_BUILD_EVERYTHING "Build with most options enabled (except HepMC3 and documentation)" OFF)
# core related options
set(ACTS_PARAMETER_DEFINITIONS_HEADER "" CACHE FILEPATH "Use a different (track) parameter definitions header")
set(ACTS_SOURCELINK_SBO_SIZE "" CACHE STRING "Customize the SBO size used by SourceLink")
option(ACTS_FORCE_ASSERTIONS "Force assertions regardless of build type" OFF)
# external library options
option(ACTS_USE_SYSTEM_LIBS "Use system libraries by default" OFF)
# plugins related options
option(ACTS_BUILD_PLUGIN_AUTODIFF "Build the autodiff plugin" OFF)
option(ACTS_USE_SYSTEM_AUTODIFF "Use autodiff provided by the system instead of the bundled version" ${ACTS_USE_SYSTEM_LIBS})
option(ACTS_USE_SYSTEM_ACTSVG "Use the ActSVG system library" ${ACTS_USE_SYSTEM_LIBS})
option(ACTS_BUILD_PLUGIN_ACTSVG "Build SVG display plugin" OFF)
option(ACTS_BUILD_PLUGIN_CUDA "Build CUDA plugin" OFF)
option(ACTS_BUILD_PLUGIN_DD4HEP "Build DD4hep plugin" OFF)
option(ACTS_BUILD_PLUGIN_PODIO "Build Podio plugin" OFF)
option(ACTS_BUILD_PLUGIN_EDM4HEP "Build EDM4hep plugin" OFF)
option(ACTS_BUILD_PLUGIN_FPEMON "Build FPE monitoring plugin" OFF)
option(ACTS_BUILD_PLUGIN_GEANT4 "Build Geant4 plugin" OFF)
option(ACTS_BUILD_PLUGIN_EXATRKX "Build the Exa.TrkX plugin" OFF)
option(ACTS_EXATRKX_ENABLE_ONNX "Build the Onnx backend for the exatrkx plugin" OFF)
option(ACTS_EXATRKX_ENABLE_TORCH "Build the torchscript backend for the exatrkx plugin" ON)
option(ACTS_BUILD_PLUGIN_IDENTIFICATION "Build Identification plugin" OFF)
option(ACTS_BUILD_PLUGIN_JSON "Build json plugin" OFF)
option(ACTS_USE_SYSTEM_NLOHMANN_JSON "Use nlohmann::json provided by the system instead of the bundled version" ${ACTS_USE_SYSTEM_LIBS})
option(ACTS_BUILD_PLUGIN_LEGACY "Build legacy plugin" OFF)
option(ACTS_BUILD_PLUGIN_ONNX "Build ONNX plugin" OFF)
option(ACTS_BUILD_PLUGIN_MLPACK "Build MLpack plugin" OFF)
option(ACTS_SETUP_VECMEM "Explicitly set up vecmem for the project" OFF)
option(ACTS_USE_SYSTEM_VECMEM "Use a system-provided vecmem installation" ${ACTS_USE_SYSTEM_LIBS})
option(ACTS_BUILD_PLUGIN_SYCL "Build SYCL plugin" OFF)
option(ACTS_BUILD_PLUGIN_TGEO "Build TGeo plugin" OFF)
# fatras related options
option(ACTS_BUILD_FATRAS "Build FAst TRAcking Simulation package" OFF)
option(ACTS_BUILD_FATRAS_GEANT4 "Build Geant4 Fatras package" OFF)
# alignment related options
option(ACTS_BUILD_ALIGNMENT "Build Alignment package" OFF)
# examples related options
option(ACTS_BUILD_EXAMPLES "Build standalone examples" OFF)
option(ACTS_BUILD_EXAMPLES_DD4HEP "Build DD4hep-based code in the examples" OFF)
option(ACTS_BUILD_EXAMPLES_EDM4HEP "Build EDM4hep-based code in the examples" OFF)
option(ACTS_BUILD_EXAMPLES_EXATRKX "Build the Exa.TrkX example code" OFF)
option(ACTS_BUILD_EXAMPLES_GEANT4 "Build Geant4-based code in the examples" OFF)
option(ACTS_BUILD_EXAMPLES_HEPMC3 "Build HepMC3-based code in the examples" OFF)
option(ACTS_BUILD_EXAMPLES_PYTHIA8 "Build Pythia8-based code in the examples" OFF)
option(ACTS_BUILD_EXAMPLES_PYTHON_BINDINGS "Build python bindings for the examples" OFF)
option(ACTS_BUILD_EXAMPLES_BINARIES "Build the examples binaries (deprecated)" OFF)
option(ACTS_USE_SYSTEM_PYBIND11 "Use a system installation of pybind11" ${ACTS_USE_SYSTEM_LIBS} )
option(ACTS_USE_EXAMPLES_TBB "Use Threading Building Blocks library in the examples" ON)
option(ACTS_BUILD_ANALYSIS_APPS "Build Analysis applications in the examples" OFF)
# test related options
option(ACTS_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(ACTS_BUILD_INTEGRATIONTESTS "Build integration tests" OFF)
option(ACTS_BUILD_UNITTESTS "Build unit tests" OFF)
option(ACTS_BUILD_NONCOMPILE_TESTS "Build tests that check build failure invariants" OFF)
option(ACTS_RUN_CLANG_TIDY "Run clang-tidy static analysis" OFF)
# other options
option(ACTS_BUILD_DOCS "Build documentation" OFF)
option(ACTS_SETUP_BOOST "Explicitly set up Boost for the project" ON)
option(ACTS_USE_SYSTEM_BOOST "Use a system-provided boost" ON)
option(ACTS_SETUP_EIGEN3 "Explicitly set up Eigen3 for the project" ON)
option(ACTS_USE_SYSTEM_EIGEN3 "Use a system-provided eigen3" ON)
option(ACTS_BUILD_ODD "Build the OpenDataDetector" OFF)
# profiling related optios
option(ACTS_ENABLE_CPU_PROFILING "Enable CPU profiling using gperftools" OFF)
option(ACTS_ENABLE_MEMORY_PROFILING "Enable memory profiling using gperftools" OFF)
set(ACTS_GPERF_INSTALL_DIR "" CACHE STRING "Hint to help find gperf if profiling is enabled")

option(ACTS_ENABLE_LOG_FAILURE_THRESHOLD "Enable failing on log messages with level above certain threshold" OFF)
set(ACTS_LOG_FAILURE_THRESHOLD "" CACHE STRING "Log level above which an exception should be automatically thrown. If ACTS_ENABLE_LOG_FAILURE_THRESHOLD is set and this is unset, this will enable a runtime check of the log level.")

# handle option inter-dependencies and the everything flag
# NOTE: ordering is important here. dependencies must come before dependees
include(ActsOptionHelpers)
# options without inter-dependencies
set_option_if(ACTS_BUILD_BENCHMARKS ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_UNITTESTS ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_NONCOMPILE_TESTS ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_INTEGRATIONTESTS ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_EXAMPLES_DD4HEP ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_EXAMPLES_GEANT4 ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_EXAMPLES_HEPMC3 ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_EXAMPLES_PYTHIA8 ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_FATRAS_GEANT4 ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_FATRAS ACTS_BUILD_FATRAS_GEANT4)
set_option_if(ACTS_BUILD_ALIGNMENT ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_EXAMPLES_EDM4HEP ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_ANALYSIS_APPS ACTS_BUILD_EVERYTHING)

# any examples component activates the general examples option
set_option_if(
  ACTS_BUILD_EXAMPLES
  ACTS_BUILD_EXAMPLES_DD4HEP
  OR ACTS_BUILD_EXAMPLES_EDM4HEP
  OR ACTS_BUILD_EXAMPLES_GEANT4
  OR ACTS_BUILD_EXAMPLES_HEPMC3
  OR ACTS_BUILD_EXAMPLES_PYTHIA8
  OR ACTS_BUILD_EXAMPLES_EXATRKX
  OR ACTS_BUILD_EXAMPLES_PYTHON_BINDINGS
  OR ACTS_BUILD_EXAMPLES_BINARIES
  OR ACTS_BUILD_EVERYTHING)
# core plugins might be required by examples or depend on each other
set_option_if(
  ACTS_BUILD_PLUGIN_DD4HEP
  ACTS_BUILD_EXAMPLES_DD4HEP OR ACTS_BUILD_EVERYTHING OR
  ACTS_BUILD_EXAMPLES_EDM4HEP)
set_option_if(
  ACTS_BUILD_PLUGIN_EDM4HEP
  ACTS_BUILD_EXAMPLES_EDM4HEP OR ACTS_BUILD_EVERYTHING)
set_option_if(
  ACTS_BUILD_PLUGIN_PODIO
  ACTS_BUILD_EVERYTHING)
set_option_if(
    ACTS_BUILD_PLUGIN_GEANT4
    ACTS_BUILD_EXAMPLES_GEANT4 OR ACTS_BUILD_EVERYTHING)  
set_option_if(
  ACTS_BUILD_PLUGIN_TGEO
  ACTS_BUILD_PLUGIN_DD4HEP OR ACTS_BUILD_EXAMPLES OR ACTS_BUILD_EVERYTHING)
set_option_if(
  ACTS_BUILD_PLUGIN_IDENTIFICATION
  ACTS_BUILD_PLUGIN_TGEO OR ACTS_BUILD_PLUGIN_PODIO OR ACTS_BUILD_EXAMPLES OR ACTS_BUILD_EVERYTHING)
set_option_if(
  ACTS_BUILD_PLUGIN_JSON
  ACTS_BUILD_EXAMPLES OR ACTS_BUILD_EVERYTHING)
set_option_if(
  ACTS_BUILD_FATRAS
  ACTS_BUILD_EXAMPLES OR ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_PLUGIN_LEGACY ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_PLUGIN_AUTODIFF ACTS_BUILD_EVERYTHING)
set_option_if(ACTS_BUILD_PLUGIN_EXATRKX ACTS_BUILD_EXAMPLES_EXATRKX)
set_option_if(ACTS_BUILD_PLUGIN_FPEMON 
  ACTS_BUILD_EXAMPLES OR ACTS_BUILD_EVERYTHING)

# feature tests
include(CheckCXXSourceCompiles)

# function that tests if the root installation is compatible
function(check_root_compatibility)
  get_target_property(ROOT_INCLUDE_DIR ROOT::Core INTERFACE_INCLUDE_DIRECTORIES)
  set(CMAKE_REQUIRED_INCLUDES ${ROOT_INCLUDE_DIR})
  #yolo
  #check_cxx_source_compiles("#include <string>\n#include <TString.h>\nint main(){}" ROOT_COMPATIBILITY_CHECK)
  #if(NOT ROOT_COMPATIBILITY_CHECK)
  #  message(FATAL_ERROR "Root installation is misconfigured. Ensure that your Root installation was compiled.")
  #endif()
endfunction()

# additional configuration and tools
include(GNUInstallDirs) # GNU-like installation paths, e.g. lib/, include/, ...
include(ActsCompilerOptions) # default compile options
include(ActsComponentsHelpers) # handle components via add_..._if commands
include(ActsStaticAnalysis)

# place build products in `<build>/bin` and `<build>/lib` for simple use
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

# This needs to happen before we set up any targets
if(ACTS_FORCE_ASSERTIONS)
  message(STATUS "Injecting headers to force assertions. This can have side-effects, USE WITH CAUTION!")
  include_directories(BEFORE SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/cmake/assert_include)
endif()

# minimal dependency versions. they are defined here in a single place so
# they can be easily upgraded, although they might not be used if the
# dependency is included via `add_subdirectory(...)`.
set(_acts_actsvg_version 0.4.35)
set(_acts_autodiff_version 0.6)
set(_acts_boost_version 1.71.0)
set(_acts_dd4hep_version 1.21)
set(_acts_edm4hep_version 0.7)
set(_acts_podio_version 0.16)
set(_acts_doxygen_version 1.9.4)
set(_acts_eigen3_version 3.3.7)
set(_acts_hepmc3_version 3.2.1)
set(_acts_nlohmanjson_version 3.2.0)
set(_acts_onnxruntime_version 1.12.0)
set(_acts_mlpack_version 3.1.1)
set(_acts_root_version 6.20)
set(_acts_tbb_version 2020.1)

# recommended dependency version. if there is an opportunity to reach
# this version we will try so.
set(_acts_boost_recommended_version 1.78.0)

# required packages
if (ACTS_SETUP_BOOST)
  if (ACTS_USE_SYSTEM_BOOST)
    # NOTE FindBoost.cmake looks for BoostConfig.cmake first, before running it's own logic.

    # Enable both program_options and unit_test_framework to reduce complexity
    # Also Cuda tests seem to use program_options
    if( ACTS_BUILD_EXAMPLES_BINARIES OR ACTS_BUILD_ANALYSIS_APPS OR
        ACTS_BUILD_UNITTESTS OR ACTS_BUILD_INTEGRATIONTESTS OR ACTS_BUILD_BENCHMARKS )
      find_package(Boost ${_acts_boost_version} REQUIRED COMPONENTS program_options unit_test_framework)
    else()
      find_package(Boost ${_acts_boost_version} REQUIRED COMPONENTS)
    endif()

    if(Boost_VERSION VERSION_LESS _acts_boost_recommended_version)
      message(WARNING "Found Boost ${Boost_VERSION} - recommended is at least ${_acts_boost_recommended_version}")
    endif()
  else()
    add_subdirectory(thirdparty/boost)
  endif()
endif()

if (ACTS_SETUP_EIGEN3)
  if (ACTS_USE_SYSTEM_EIGEN3)
    find_package(Eigen3 ${_acts_eigen3_version} REQUIRED CONFIG)
  else()
    add_subdirectory(thirdparty/eigen3)
  endif()
endif()

if (ACTS_SETUP_VECMEM)
  if (ACTS_USE_SYSTEM_VECMEM)
    find_package(vecmem REQUIRED)
  else()
    add_subdirectory(thirdparty/vecmem)
  endif()
endif()

find_package(Filesystem REQUIRED)

# the `<project_name>_VERSION` variables set by `setup(... VERSION ...)` have
# only local scope, i.e. they are not accessible her for dependencies added
# via `add_subdirectory`. this overrides the `project(...)` funcion for
# sub-projects such that the resulting `<project_name>_VERSION` has
# global scope and is accessible within the main project later on.
cmake_policy(SET CMP0048 NEW)
macro(project)
  _project(${ARGN})
  set(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION}" CACHE INTERNAL "")
endmacro()

# CUDA settings are collected here in a macro, so that they can be reused by different plugins
macro(enable_cuda)
  enable_language(CUDA)
  set(CMAKE_CUDA_STANDARD 14 CACHE STRING "CUDA C++ standard to use")
  set(CMAKE_CUDA_STANDARD_REQUIRED ON CACHE BOOL
    "Force the C++ standard requirement")
  if(NOT CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES "35;52;75" CACHE STRING
        "CUDA architectures to generate code for")
  endif()
  set(CMAKE_CUDA_FLAGS_DEBUG "${CMAKE_CUDA_FLAGS_DEBUG} -g -G")
  set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --extended-lambda")
endmacro()

# optional packages
#
# find packages explicitly for each component even if this means searching for
# the same package twice. This avoids having complex if/else trees to sort out
# when a particular package is actually needed.

# plugin dependencies
if(ACTS_BUILD_PLUGIN_ACTSVG)
  if(ACTS_USE_SYSTEM_ACTSVG)
    message(STATUS "Using system installation of actsvg")
    find_package(actsvg ${_acts_actsvg_version} REQUIRED CONFIG)
  else()
    add_subdirectory(thirdparty/actsvg)
  endif()
endif()
if(ACTS_BUILD_PLUGIN_AUTODIFF)
  if(ACTS_USE_SYSTEM_AUTODIFF)
    find_package(autodiff ${_acts_autodiff_version} CONFIG REQUIRED)
    message(STATUS "Using system installation of autodiff")
  else()
    add_subdirectory(thirdparty/autodiff)
  endif()
endif()
if(ACTS_BUILD_PLUGIN_CUDA)
  enable_cuda()
endif()
if(ACTS_BUILD_PLUGIN_DD4HEP)
  # Explicitly find python so we can more easily override the version
  find_package(Python 3.8 REQUIRED COMPONENTS Interpreter)
  find_package(DD4hep ${_acts_dd4hep_version} REQUIRED CONFIG COMPONENTS DDCore DDDetectors)
endif()
if(ACTS_BUILD_PLUGIN_JSON)
  if(ACTS_USE_SYSTEM_NLOHMANN_JSON)
    message(STATUS "Using system installation of nlohmann::json")
    find_package(nlohmann_json ${_acts_nlohmanjson_version} REQUIRED CONFIG)
  else()
    add_subdirectory(thirdparty/nlohmann_json)
  endif()
endif()
if(ACTS_BUILD_PLUGIN_MLPACK)
  find_package(mlpack ${_acts_mlpack_version} REQUIRED)
endif()
if(ACTS_BUILD_PLUGIN_SYCL)
  find_package(SYCL REQUIRED)
endif()
if(ACTS_BUILD_PLUGIN_TGEO)
  find_package(ROOT ${_acts_root_version} REQUIRED CONFIG COMPONENTS Geom)
  check_root_compatibility()
endif()
if(ACTS_BUILD_ANALYSIS_APPS)
  find_package(ROOT ${_acts_root_version} REQUIRED CONFIG COMPONENTS Geom)
  check_root_compatibility()
endif()
if(ACTS_BUILD_PLUGIN_EXATRKX)
  find_package(Torch REQUIRED)
  find_package(CUDAToolkit)
  if(CUDAToolkit_FOUND)
    enable_cuda()
    add_subdirectory(thirdparty/FRNN)
  else()
    message(WARNING "CUDA not found, build Exa.TrkX plugin only for CPU")
  endif()
  if(NOT (ACTS_EXATRKX_ENABLE_ONNX OR ACTS_EXATRKX_ENABLE_TORCH))
    message(FATAL_ERROR 
      "When building the Exa.TrkX plugin, at least one of ACTS_EXATRKX_ENABLE_ONNX \
      and ACTS_EXATRKX_ENABLE_TORCHSCRIPT must be enabled."
    )
  endif()
  if(ACTS_EXATRKX_ENABLE_ONNX)
    find_package(cugraph REQUIRED)
  endif()
  if(ACTS_EXATRKX_ENABLE_TORCH)
    find_package(TorchScatter REQUIRED)
  endif()
endif()
if(ACTS_BUILD_PLUGIN_ONNX OR ACTS_EXATRKX_ENABLE_ONNX)
  find_package(OnnxRuntime ${_acts_onnxruntime_version} REQUIRED)
endif()
if(ACTS_BUILD_PLUGIN_EDM4HEP OR ACTS_BUILD_PLUGIN_PODIO)
  find_package(podio ${_acts_podio_version} REQUIRED CONFIG)
  find_package(ROOT ${_acts_root_version} REQUIRED CONFIG COMPONENTS Core)
endif()
if(ACTS_BUILD_PLUGIN_EDM4HEP)
  find_package(EDM4HEP ${_acts_edm4hep_version} REQUIRED CONFIG)
endif()
if(ACTS_BUILD_PLUGIN_GEANT4)
  find_package(Geant4 REQUIRED CONFIG COMPONENTS gdml)
endif()

# examples dependencies
if(ACTS_BUILD_EXAMPLES)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  # for simplicity always request all potentially required components.
  find_package(ROOT ${_acts_root_version} REQUIRED CONFIG COMPONENTS Core Geom GenVector Hist Tree TreePlayer)
  check_root_compatibility()
  add_subdirectory(thirdparty/dfelibs)
endif()
if(ACTS_BUILD_EXAMPLES_PYTHON_BINDINGS)
  find_package(Python 3.8 REQUIRED COMPONENTS Interpreter Development)
  if(ACTS_USE_SYSTEM_PYBIND11) 
    find_package(pybind11 CONFIG REQUIRED)
  else()
    add_subdirectory(thirdparty/pybind11)
  endif()
endif()
if(ACTS_BUILD_EXAMPLES_DD4HEP AND ACTS_BUILD_EXAMPLES_GEANT4)
  find_package(DD4hep ${_acts_dd4hep_version} REQUIRED CONFIG COMPONENTS DDCore DDG4 DDDetectors)
elseif(ACTS_BUILD_EXAMPLES_DD4HEP)
  find_package(DD4hep ${_acts_dd4hep_version} REQUIRED CONFIG COMPONENTS DDCore DDDetectors)
endif()
if(ACTS_BUILD_EXAMPLES_HEPMC3)
  find_package(HepMC3 ${_acts_hepmc3_version} REQUIRED CONFIG)
endif()
if(ACTS_BUILD_EXAMPLES_PYTHIA8)
  find_package(Pythia8 REQUIRED)

  if(DEFINED CMAKE_CXX_STANDARD)
    if(${CMAKE_CXX_STANDARD} GREATER_EQUAL 20)
      message(WARNING "ACTS is configured to build with C++20."
        "As of version 309, Pythia8 does not compile under C++20."
        "You can find a patch file at: https://gist.github.com/paulgessinger/7a823b4015d4511cf829472291c8ddcf")
    endif()
  endif()
endif()
# other dependencies
if(ACTS_BUILD_DOCS)
  find_package(Doxygen ${_acts_doxygen_version} REQUIRED)
  find_package(Sphinx REQUIRED)
endif()

if(ACTS_CUSTOM_SCALARTYPE)
  message(STATUS "Building Acts with custom scalar type: ${ACTS_CUSTOM_SCALARTYPE}")
endif()

# core library, core plugins, and other components
add_component(Core Core)
add_subdirectory(Plugins)
add_component_if(Fatras Fatras ACTS_BUILD_FATRAS)
add_component_if(Alignment Alignment ACTS_BUILD_ALIGNMENT)

if(ACTS_BUILD_ODD)
  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/thirdparty/OpenDataDetector/CMakeLists.txt")
    add_subdirectory_if(thirdparty/OpenDataDetector ACTS_BUILD_ODD)
  else()
    message(FATAL_ERROR "ODD build was requested, but the ODD directory seems not to be present. "
                        "Did you init and update the submodule?")
  endif()
endif()


# examples
add_subdirectory_if(Examples ACTS_BUILD_EXAMPLES)

# automated tests and benchmarks
if(ACTS_BUILD_BENCHMARKS OR ACTS_BUILD_INTEGRATIONTESTS OR ACTS_BUILD_UNITTESTS)
  enable_testing() # must be set in the main CMakeLists.txt
  add_subdirectory(Tests)
endif()

# documentation
add_subdirectory_if(docs ACTS_BUILD_DOCS)


# create cmake configuration files and environment setup script
include(ActsCreatePackageConfig)
include(ActsCreateSetup)
