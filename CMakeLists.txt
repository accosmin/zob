cmake_minimum_required(VERSION 3.1)

project(nanocv)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake )

######################################################################
# external dependencies
######################################################################

option(NANOCV_WITH_ASAN         "build with address sanitizer suppport (debug, gcc)"    OFF)
option(NANOCV_WITH_TSAN         "build with thread sanitizer suppport (debug, gcc)"     OFF)

# Zlib & BZip2
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)

include_directories(SYSTEM ${ZLIB_INCLUDE_DIR})
include_directories(SYSTEM ${BZIP2_INCLUDE_DIR})

# Boost
set(Boost_USE_STATIC_LIBS        OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
set(BOOST_ALL_DYN_LINK           ON)
find_package(Boost COMPONENTS serialization program_options filesystem system unit_test_framework REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# DevIL
find_package(DevIL REQUIRED)
include_directories(SYSTEM ${IL_INCLUDE_DIR})

# LibArchive
find_package(LibArchive REQUIRED)
include_directories(SYSTEM ${LibArchive_INCLUDE_DIRS})

# OpenMP
find_package(OpenMP QUIET)
if(OPENMP_FOUND)
	set(NANOCV_OPENMP_FLAG "-fopenmp")
endif()

find_package( Eigen3 )
include_directories ( ${EIGEN3_INCLUDE_DIR} )

# Eigen
add_definitions(-DEIGEN_DONT_PARALLELIZE)

#set(CMAKE_C_COMPILER                    "/usr/bin/clang")
#set(CMAKE_CXX_COMPILER                  "/usr/bin/clang++")
#set(CMAKE_CXX_COMPILER                  "g++-4.9")

######################################################################
# compiler flags
######################################################################

include(CheckCXXCompilerFlag)

set(NANOCV_TEST_PROGRAM "int main() { return 0; }")

CHECK_CXX_COMPILER_FLAG(-std=c++14 COMPILER_SUPPORTS_CXX14)

#CHECK_CXX_COMPILER_FLAG(-fsanitize=address COMPILER_SUPPORTS_SANITIZE_ADDRESS)
#CHECK_CXX_COMPILER_FLAG(-fsanitize=undefined COMPILER_SUPPORTS_SANITIZE_UNDEFINED)
#CHECK_CXX_COMPILER_FLAG(-fsanitize=leak COMPILER_SUPPORTS_SANITIZE_LEAK)
#CHECK_CXX_COMPILER_FLAG(-fsanitize=thread COMPILER_SUPPORTS_SANITIZE_THREAD)

set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
CHECK_CXX_SOURCE_COMPILES("${NANOCV_TEST_PROGRAM}" COMPILER_SUPPORTS_SANITIZE_ADDRESS)

set(CMAKE_REQUIRED_FLAGS "-fsanitize=undefined")
CHECK_CXX_SOURCE_COMPILES("${NANOCV_TEST_PROGRAM}" COMPILER_SUPPORTS_SANITIZE_UNDEFINED)

set(CMAKE_REQUIRED_FLAGS "-fsanitize=leak")
CHECK_CXX_SOURCE_COMPILES("${NANOCV_TEST_PROGRAM}" COMPILER_SUPPORTS_SANITIZE_LEAK)

set(CMAKE_REQUIRED_FLAGS "-fsanitize=thread -fPIE -pie")
CHECK_CXX_SOURCE_COMPILES("${NANOCV_TEST_PROGRAM}" COMPILER_SUPPORTS_SANITIZE_THREAD)

######################################################################
# compilation & linking flags
######################################################################

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        message("Compiling with ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} ...")

        # check C++14
        CHECK_CXX_COMPILER_FLAG(-std=c++14 COMPILER_SUPPORTS_CXX14)

        if(NOT COMPILER_SUPPORTS_CXX14)
                message(FATAL_ERROR "The compiler has no C++14 support.")
        endif()

        # set flags
        set(CMAKE_CXX_FLAGS			"-std=c++14 -pedantic -march=native -mtune=native")
        set(CMAKE_CXX_FLAGS			"${CMAKE_CXX_FLAGS} -Wall -Wextra") #-Wconversion
        set(CMAKE_CXX_FLAGS                     "${CMAKE_CXX_FLAGS} ${NANOCV_OPENMP_FLAG}")
        set(CMAKE_CXX_FLAGS                     "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
        set(CMAKE_CXX_FLAGS_DEBUG		"-g -fno-omit-frame-pointer")
	set(CMAKE_CXX_FLAGS_RELEASE             "-O3 -DNDEBUG")         # -DEIGEN_NO_DEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO      "-O2 -g")               # -DEIGEN_NO_DEBUG")
	set(CMAKE_CXX_FLAGS_MINSIZEREL          "-Os -DNDEBUG")         # -DEIGEN_NO_DEBUG")
	set(CMAKE_EXE_LINKER_FLAGS              "-flto")

        # set address sanitizer
	if(NANOCV_WITH_ASAN)
                CHECK_CXX_COMPILER_FLAG(-fsanitize=address COMPILER_SUPPORTS_SANITIZE_ADDRESS)
                CHECK_CXX_COMPILER_FLAG(-fsanitize=undefined COMPILER_SUPPORTS_SANITIZE_UNDEFINED)

                if(COMPILER_SUPPORTS_SANITIZE_ADDRESS AND COMPILER_SUPPORTS_SANITIZE_UNDEFINED)
                        set(CMAKE_CXX_FLAGS_DEBUG       "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
                        set(CMAKE_CXX_FLAGS_DEBUG       "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
                        set(CMAKE_CXX_FLAGS_DEBUG       "${CMAKE_CXX_FLAGS_DEBUG} -fno-sanitize=vptr")
                else()
                        message(WARNING "The compiler has no address sanitizer support. Ignoring...")
                endif()

                if(COMPILER_SUPPORTS_SANITIZE_LEAK)
                        set(CMAKE_CXX_FLAGS_DEBUG       "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak")
                else()
                        message(WARNING "The compiler has no leak sanitizer support. Ignoring...")
                endif()

        # set thread sanitizer
	elseif(NANOCV_WITH_TSAN)
                if(COMPILER_SUPPORTS_SANITIZE_THREAD)
                        set(CMAKE_CXX_FLAGS_DEBUG       "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread -fPIE")
                        set(CMAKE_EXE_LINKER_FLAGS      "${CMAKE_EXE_LINKER_FLAGS} -pie")
                else()
                        message(WARNING "The compiler has no thread sanitizer support. Ignoring...")
                endif()
	endif()

else()
	message(WARNING "Compiling with an unsupported compiler ...")
endif()

# debug
if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
        add_definitions(-DNANOCV_DEBUG)
endif()

message("------------------------------------------------------------------------------" "")
message("CXX FLAGS:                    " "${CMAKE_CXX_FLAGS}")
message("CXX DEBUG FLAGS:              " "${CMAKE_CXX_FLAGS_DEBUG}")
message("CXX RELEASE FLAGS:            " "${CMAKE_CXX_FLAGS_RELEASE}")
message("CXX RELWITHDEBINFO FLAGS:     " "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message("CXX MINSIZEREL FLAGS:         " "${CMAKE_CXX_FLAGS_MINSIZEREL}")
message("CMAKE_EXE_LINKER_FLAGS:       " "${CMAKE_EXE_LINKER_FLAGS}")
message("------------------------------------------------------------------------------" "")
message("BUILD TYPE:                   " "${CMAKE_BUILD_TYPE}")
message("------------------------------------------------------------------------------" "")
message("ASAN                          " "${NANOCV_WITH_ASAN}")
message("TSAN                          " "${NANOCV_WITH_TSAN}")
message("------------------------------------------------------------------------------" "")

######################################################################
# build the library, programs & tests
######################################################################

include_directories( ${nanocv_SOURCE_DIR} )

add_subdirectory(nanocv)
add_subdirectory(apps)
add_subdirectory(tests)

