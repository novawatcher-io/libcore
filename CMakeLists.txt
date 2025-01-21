cmake_minimum_required(VERSION 3.16)

# used as a module of other projects
if(NOT DEFINED PROJECT_NAME)
    project(core LANGUAGES CXX)
    add_compile_options(-Wall -Wextra -Werror)
    # enable clang-tidy
    find_program(CLANG_TIDY clang-tidy)
    if(CLANG_TIDY)
        # set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY})
    endif()
    # enable include-what-you-use
    find_program(IWYU include-what-you-use)
    if(IWYU)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU})
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-O0 -g)
        if(SANITIZER_TYPE STREQUAL "address")
            add_compile_options(-fsanitize=address)
            add_link_options(-fsanitize=address)
        elseif(SANITIZER_TYPE STREQUAL "thread")
            add_compile_options(-fsanitize=thread -fPIE -pie)
            add_link_options(-fsanitize=thread)
        endif()
    else()
        add_compile_options(-O3)
    endif()
endif()

# options
set(libcore_FMT_PROVIDER
    "package"
    CACHE STRING "Provider of fmt library")
set_property(CACHE libcore_FMT_PROVIDER PROPERTY STRINGS "module" "package")

set(libcore_LIBEVENT_PROVIDER
    "package"
    CACHE STRING "Provider of LIBEVENT library")
set_property(CACHE libcore_LIBEVENT_PROVIDER PROPERTY STRINGS "module" "package")

set(libcore_SPDLOG_PROVIDER
    "package"
    CACHE STRING "Provider of spdlog library")
set_property(CACHE libcore_SPDLOG_PROVIDER PROPERTY STRINGS "module" "package")

option(libcore_BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(ZLIB REQUIRED)

# libcgroup support pkg-config, but not cmake
find_package(PkgConfig)
pkg_check_modules(LIBCGROUP REQUIRED libcgroup)

if(libcore_FMT_PROVIDER STREQUAL "module")
    add_subdirectory(third_party/fmt)
else()
    find_package(fmt CONFIG REQUIRED)
endif()

if(libcore_LIBEVENT_PROVIDER STREQUAL "module")
    set(EVENT__DISABLE_TESTS ON)
    add_subdirectory(third_party/libevent)
else()
    find_package(Libevent CONFIG REQUIRED)
endif()

if(libcore_SPDLOG_PROVIDER STREQUAL "module")
    add_subdirectory(third_party/spdlog)
else()
    add_compile_definitions(SPDLOG_FMT_EXTERNAL)
    find_package(spdlog CONFIG REQUIRED)
endif()

set(libcore_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core/include)
add_subdirectory(core)

add_library(core INTERFACE)
target_link_libraries(core INTERFACE libcore::common libcore::component libcore::event libcore::process libcore::http
                                     libcore::os)

if(libcore_BUILD_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)
    add_subdirectory(test)
endif()
