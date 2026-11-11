cmake_minimum_required(VERSION 3.22)
project(Sundrift VERSION 0.1.3 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

# Uses JUCE 8 LTS security release. You can alternatively set -DJUCE_DIR=<path>.
if(DEFINED JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
  add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)
else()
  FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
                       GIT_TAG 8.0.17-security GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(juce)
endif()

option(SUNDRIFT_WITH_NAM "Enable optional NAM A2 capture loading" ON)

juce_add_plugin(Sundrift
  COMPANY_NAME "Sundrift Audio"
  BUNDLE_ID "audio.sundrift.sundrift"
  PLUGIN_MANUFACTURER_CODE Sdft
  PLUGIN_CODE Sdrf
  FORMATS VST3
  PRODUCT_NAME "Sundrift"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Fx Modulation Reverb Delay
)
juce_generate_juce_header(Sundrift)
target_sources(Sundrift PRIVATE
  Source/PluginProcessor.cpp Source/PluginProcessor.h
  Source/PluginEditor.cpp Source/PluginEditor.h
  Source/ToneEngine.h
)
target_compile_definitions(Sundrift PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(Sundrift PRIVATE
  juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags
)

if(SUNDRIFT_WITH_NAM)
  # Build the verified A2-capable version of NeuralAmpModelerCore as an internal static library.
  # Git clone recursively fetches the Eigen and nlohmann-json submodules.
  FetchContent_Declare(namcore GIT_REPOSITORY https://github.com/sdatkinson/NeuralAmpModelerCore.git
    GIT_TAG v0.5.2 GIT_SHALLOW TRUE GIT_SUBMODULES_RECURSE TRUE)
  FetchContent_GetProperties(namcore)
  if(NOT namcore_POPULATED)
    FetchContent_Populate(namcore)
  endif()
  file(GLOB_RECURSE NAM_SOURCES CONFIGURE_DEPENDS "${namcore_SOURCE_DIR}/NAM/*.cpp")
  add_library(sundrift_nam STATIC ${NAM_SOURCES})
  target_include_directories(sundrift_nam PUBLIC "${namcore_SOURCE_DIR}"
    "${namcore_SOURCE_DIR}/Dependencies/eigen"
    "${namcore_SOURCE_DIR}/Dependencies/nlohmann")
  target_compile_definitions(sundrift_nam PUBLIC NAM_SAMPLE_FLOAT NAM_ENABLE_A2_FAST)
  target_compile_features(sundrift_nam PUBLIC cxx_std_20)
  target_link_libraries(Sundrift PRIVATE sundrift_nam)
  target_compile_definitions(Sundrift PRIVATE SUNDRIFT_WITH_NAM)
endif()

option(SUNDRIFT_BUILD_TESTS "Build dependency-free DSP tests" OFF)
if(SUNDRIFT_BUILD_TESTS)
  enable_testing()
  add_executable(sundrift_tests tests/test_engine.cpp)
  add_test(NAME DSP COMMAND sundrift_tests)

  # Runs against the real JUCE processor/editor, catching GUI startup crashes
  # that the dependency-free sound-engine tests cannot see.
  if(WIN32)
    add_executable(sundrift_host_smoke tests/host_smoke.cpp)
    target_include_directories(sundrift_host_smoke PRIVATE
      Source $<TARGET_PROPERTY:Sundrift,INCLUDE_DIRECTORIES>)
    target_compile_definitions(sundrift_host_smoke PRIVATE
      $<TARGET_PROPERTY:Sundrift,COMPILE_DEFINITIONS>)
    target_compile_options(sundrift_host_smoke PRIVATE
      $<TARGET_PROPERTY:Sundrift,COMPILE_OPTIONS>)
    target_link_libraries(sundrift_host_smoke PRIVATE
      Sundrift juce::juce_audio_utils juce::juce_dsp)
    add_test(NAME HostLifecycle COMMAND sundrift_host_smoke)
  endif()
endif()
