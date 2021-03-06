########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosSDR CXX)

if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    find_package(Pothos "0.4.0" CONFIG REQUIRED)
else()
    find_package(Pothos CONFIG REQUIRED) #in-tree build
endif()

########################################################################
## Find SoapySDR
########################################################################
find_package(SoapySDR CONFIG)

if (SoapySDR_FOUND)
    message(STATUS "SoapySDR_VERSION: ${SoapySDR_VERSION}")
    message(STATUS "SoapySDR_INCLUDE_DIRS: ${SoapySDR_INCLUDE_DIRS}")
    message(STATUS "SoapySDR_LIBRARIES: ${SoapySDR_LIBRARIES}")
else()
    message(WARNING "Pothos SDR blocks require SoapySDR, skipping...")
endif()

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_SDR "Enable Pothos SDR component" ON "Pothos_FOUND;SoapySDR_FOUND" OFF)
add_feature_info(SDR ENABLE_SDR "SDR source and sink blocks for radio hardware")
if (NOT ENABLE_SDR)
    return()
endif()

########################################################################
## Generate block descriptions
########################################################################
set(TITLE "Source")
set(CATEGORY "/Sources")
set(PATH "/sdr/source")
set(XCVR "receiver")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SDRBlockDesc.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/SDRSourceDesc.hpp
@ONLY)

set(TITLE "Sink")
set(CATEGORY "/Sinks")
set(PATH "/sdr/sink")
set(XCVR "transmitter")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SDRBlockDesc.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/SDRSinkDesc.hpp
@ONLY)

########################################################################
## Build and install module
########################################################################
include_directories(${SoapySDR_INCLUDE_DIRS})
POTHOS_MODULE_UTIL(
    TARGET SDRSupport
    SOURCES
        SDRBlock.cpp
        SDRSource.cpp
        SDRSink.cpp
        DemoController.cpp
        Logger.cpp
        SDRInfo.cpp
        SDRBlockThread.cpp
    LIBRARIES ${SoapySDR_LIBRARIES}
    DESTINATION sdr
    DOC_SOURCES
        DemoController.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/SDRSourceDesc.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/SDRSinkDesc.hpp
)

########################################################################
## Utility blocks
########################################################################
POTHOS_MODULE_UTIL(
    TARGET SDRUtilityBlocks
    SOURCES
        ChannelAligner.cpp
        RandomDropper.cpp
        TxBurstTimer.cpp
    DESTINATION sdr
    ENABLE_DOCS
)
