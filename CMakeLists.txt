# ============================================================================
# CMakeLists.txt — Reactive Framework (RxCpp version)
# ============================================================================
cmake_minimum_required(VERSION 3.20)

project(ReactiveFramework
    VERSION     2.0.0
    DESCRIPTION "Reactive OOP Stream Processing with RxCpp"
    LANGUAGES   CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# RxCpp — dùng vcpkg hoặc external
# ---------------------------------------------------------------------------
find_package(rxcpp CONFIG QUIET)

if (NOT rxcpp_FOUND)
    # Fallback: dùng external/RxCpp
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/RxCpp/Rx/v2/src/rxcpp/rx.hpp)
        set(RXCPP_INCLUDE_DIR
            ${CMAKE_CURRENT_SOURCE_DIR}/external/RxCpp/Rx/v2/src)
        message(STATUS "RxCpp: dung external/RxCpp (header-only)")
    else()
        message(FATAL_ERROR
            "Khong tim thay RxCpp.\n"
            "Cach 1: cai qua vcpkg: vcpkg install rxcpp:x64-windows\n"
            "Cach 2: clone: git clone --recursive https://github.com/ReactiveX/RxCpp.git external/RxCpp")
    endif()
endif()

# ---------------------------------------------------------------------------
# Threads
# ---------------------------------------------------------------------------
find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------
# Interface library
# ---------------------------------------------------------------------------
add_library(reactive_framework INTERFACE)

target_include_directories(reactive_framework INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (RXCPP_INCLUDE_DIR)
    target_include_directories(reactive_framework INTERFACE ${RXCPP_INCLUDE_DIR})
endif()

# CHỈ link Threads — KHÔNG link rxcpp (header-only)
target_link_libraries(reactive_framework INTERFACE Threads::Threads)

target_compile_features(reactive_framework INTERFACE cxx_std_20)

# ---------------------------------------------------------------------------
# Executable chính
# ---------------------------------------------------------------------------
add_executable(reactive_demo src/main.cpp)
target_link_libraries(reactive_demo PRIVATE reactive_framework)

# ---------------------------------------------------------------------------
# Thông tin
# ---------------------------------------------------------------------------
message(STATUS "============================================")
message(STATUS "Reactive Framework (RxCpp) v${PROJECT_VERSION}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:     ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "RxCpp include: ${RXCPP_INCLUDE_DIR}")
message(STATUS "============================================")