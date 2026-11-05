cmake_minimum_required(VERSION 3.20)
Include(FetchContent)

# Host tests of Ch32UsbDeviceController. They run on the build machine, with
# a fake hw::UsbDevice (stubs/UsbDevice.h) and the real libArT USB stack.
#
#   cmake -S tests -B build-host -DLIBART_DIR=<libArT checkout>
#   cmake --build build-host && ctest --test-dir build-host --output-on-failure

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.12
)
set(DOCTEST_NO_INSTALL ON)
FetchContent_MakeAvailable(doctest)

# Header-only. libArT's usb::CdcAcmFunction keeps its queues in
# etl::circular_buffer. Same tag as libArT.
FetchContent_Declare(
    etl
    GIT_REPOSITORY https://github.com/ETLCPP/etl.git
    GIT_TAG 20.41.7
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(etl)

project(art-ch32-tests C CXX)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif ()

include(CTest)

# The stubs stay first on the include path, so they win over the libArT headers
# with the same name (utils/RingBuffer.h, utils/Span.h).
set(LIBART_DIR "" CACHE PATH "libArT checkout for the host tests")
if (NOT EXISTS "${LIBART_DIR}/src/usb/DeviceController.h")
    message(FATAL_ERROR "libArT with USB support not found. Set LIBART_DIR to a libArT checkout.")
endif ()

add_executable(art-ch32-tests
    TestCh32UsbDeviceController.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/usb/Ch32UsbDeviceController.cpp"
    "${LIBART_DIR}/src/usb/Device.cpp"
    "${LIBART_DIR}/src/usb/CdcAcmFunction.cpp"
    "${LIBART_DIR}/src/streams/Readable.cpp"
    "${LIBART_DIR}/src/utils/tfp_printf.cpp"
)

target_include_directories(art-ch32-tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_SOURCE_DIR}/stubs"
    "${LIBART_DIR}/src"
    "${LIBART_DIR}/tests/usb"
)

target_link_libraries(art-ch32-tests PRIVATE doctest::doctest etl::etl)

# As art-core: 1 (std::expected and std::span) at C++23, 0 (own types and the
# span of span-compat) below.
if (CMAKE_CXX_STANDARD LESS 23)
    include("${LIBART_DIR}/cmake/span-compat.cmake")
    set(SPAN_COMPAT_USE_STD_SPAN OFF CACHE STRING "Span type of span-compat: AUTO, ON (std::span) or OFF (own)")
    FetchContent_Declare(
        span_compat
        GIT_REPOSITORY ${ART_SPAN_COMPAT_GIT_REPOSITORY}
        GIT_TAG ${ART_SPAN_COMPAT_GIT_TAG}
    )
    FetchContent_MakeAvailable(span_compat)
    target_link_libraries(art-ch32-tests PRIVATE span-compat::span-compat)
    target_compile_definitions(art-ch32-tests PRIVATE ART_USE_STD_EXPECTED_SPAN=0)
else ()
    target_compile_definitions(art-ch32-tests PRIVATE ART_USE_STD_EXPECTED_SPAN=1)
endif ()

add_test(NAME art-ch32-tests COMMAND art-ch32-tests)
