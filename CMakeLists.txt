cmake_minimum_required(VERSION 3.0)

# Add in some lovely flags.  use no-unused-function because of kseq
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g -Wno-unused-function -O3")

# This is a C project called sampling!
project("sampling" C)

set(SMPL_VERSION_MAJOR 0)
set(SMPL_VERSION_MINOR 1)
set(SMPL_VERSION_PATCH 5)
set(SMPL_VERSION_STRING "${SMPL_VERSION_MAJOR}.${SMPL_VERSION_MINOR}.${SMPL_VERSION_PATCH}")

include(GNUInstallDirs)

# Project dirs
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(HEADER_DIR ${PROJECT_SOURCE_DIR}/include)
set(CONFIG_DIR ${PROJECT_SOURCE_DIR}/cmake_config)

# Vendor dirs
set(VENDOR_DIR ${PROJECT_SOURCE_DIR}/vendor)
set(VENDOR_KSEQ_DIR ${VENDOR_DIR}/kseq)
set(VENDOR_TOMMY_DIR ${VENDOR_DIR}/tommyds)
set(VENDOR_PCG_DIR ${VENDOR_DIR}/pcg)

# Source globs
file(GLOB SMPL_SOURCES ${SOURCE_DIR}/*.c)
file(GLOB KSEQ_SOURCES ${VENDOR_KSEQ_DIR}/*.c)
file(GLOB TOMMY_SOURCES ${VENDOR_TOMMY_DIR}/*.c)

# TODO technically we don't need all of these, only the 32 bit basic generators and the entropy_getbytes function.
file(GLOB PCG_SOURCES ${VENDOR_PCG_DIR}/src/*.c)


# We want to include the headers
include_directories(${HEADER_DIR})
include_directories(${VENDOR_KSEQ_DIR})
include_directories(${VENDOR_TOMMY_DIR})
include_directories(${VENDOR_PCG_DIR}/include)
include_directories(${VENDOR_PCG_DIR}/extras) # for the entropy.h


# Configure files
configure_file("${CONFIG_DIR}/sampling.cmake_config.h" "${HEADER_DIR}/sampling.h")
configure_file("${CONFIG_DIR}/README.cmake_config.md" "${PROJECT_SOURCE_DIR}/README.md")
configure_file("${CONFIG_DIR}/test_sample_seqs.cmake_config.sh" "${PROJECT_SOURCE_DIR}/test/test_sample_seqs.sh")

# Here's the program....
add_executable(sample_seqs ${SOURCE_DIR}/sample_seqs.c ${KSEQ_SOURCES} ${TOMMY_SOURCES} ${PCG_SOURCES} ${VENDOR_PCG_DIR}/extras/entropy.c)

# Install with make install (installs relative to $<INSTALL_PREFIX>)
install(TARGETS sample_seqs
        RUNTIME DESTINATION bin)

# Libraries we need....

## Check and see if OpenMP is available
#find_package(OpenMP)
#if (OPENMP_FOUND)
#    option(OPENMP "OpenMP support" ON)
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#else ()
#    option(OPENMP "OpenMP support" OFF)
#endif ()

# Check and see if zlib is available.
find_package(ZLIB)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(sample_seqs PRIVATE ${ZLIB_LIBRARIES})
else ()
    message(FATAL_ERROR "zlib library not found")
endif ()

# Find the rya library
find_library(RYA_FILE_LIB "rya_file")
find_library(RYA_FORMAT_LIB "rya_format")

find_path(RYA_H_PATH "rya.h")
find_path(RYA_FILE_H_PATH "rya_file.h")
find_path(RYA_FORMAT_H_PATH "rya_format.h")

if (RYA_H_PATH AND RYA_FILE_LIB AND RYA_FILE_H_PATH AND RYA_FORMAT_LIB AND RYA_FORMAT_H_PATH)
    # Link the rya libraries.
    target_link_libraries(sample_seqs PRIVATE ${RYA_FILE_LIB})
    target_link_libraries(sample_seqs PRIVATE ${RYA_FORMAT_LIB})

    # Include the rya headers.
    include_directories(${RYA_H_PATH} ${RYA_FILE_H_PATH} ${RYA_FORMAT_H_PATH})
else ()
    message(FATAL_ERROR "rya library not found")
endif ()

# Testing!
enable_testing()
add_test(NAME test_sample_seqs
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMAND test/test_sample_seqs.sh)