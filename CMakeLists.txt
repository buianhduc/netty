cmake_minimum_required(VERSION 3.20)

project(nes_emulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 CONFIG REQUIRED)

add_library(nes_core
    src/common/Color.cpp
    src/common/Color.h
    src/internal/bus.cpp
    src/internal/cpu.cpp
    src/internal/cpu_stack.cpp
    src/internal/opcodes.cpp
        src/internal/cpu_opcode_impl.cpp
        src/internal/rom.cpp
        src/internal/rom.h
        src/internal/ppu/NesPPU.cpp
        src/internal/ppu/NesPPU.h
        src/internal/ppu/registers/ControllerRegister.cpp
        src/internal/ppu/registers/ControllerRegister.h
        src/internal/ppu/registers/DataRegister.cpp
        src/internal/ppu/registers/DataRegister.h
        src/internal/ppu/registers/MaskRegister.cpp
        src/internal/ppu/registers/MaskRegister.h
        src/internal/ppu/registers/OamAddressRegister.cpp
        src/internal/ppu/registers/OamAddressRegister.h
        src/internal/ppu/registers/OamDataRegister.cpp
        src/internal/ppu/registers/OamDataRegister.h
        src/internal/ppu/registers/OamDmaRegister.cpp
        src/internal/ppu/registers/OamDmaRegister.h
        src/internal/ppu/registers/PPURegister.cpp
        src/internal/ppu/registers/PPURegister.h
        src/internal/ppu/registers/ScrollRegister.cpp
        src/internal/ppu/registers/ScrollRegister.h
        src/internal/ppu/registers/StatusRegister.cpp
        src/internal/ppu/registers/StatusRegister.h
        src/internal/ppu/registers/AddrRegister.cpp
        src/internal/ppu/registers/AddrRegister.h
        src/common/Frame.cpp
        src/common/Frame.h
        src/internal/Joypad.cpp
        src/internal/Joypad.h
)

target_include_directories(nes_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(nes_emulator
    src/main.cpp
        src/internal/cpu_opcode_impl.cpp
        src/internal/rom.cpp
        src/internal/rom.h
        src/internal/ppu/NesPPU.cpp
        src/internal/ppu/NesPPU.h
        src/internal/ppu/registers/ControllerRegister.cpp
        src/internal/ppu/registers/ControllerRegister.h
        src/internal/ppu/registers/DataRegister.cpp
        src/internal/ppu/registers/DataRegister.h
        src/internal/ppu/registers/MaskRegister.cpp
        src/internal/ppu/registers/MaskRegister.h
        src/internal/ppu/registers/OamAddressRegister.cpp
        src/internal/ppu/registers/OamAddressRegister.h
        src/internal/ppu/registers/OamDataRegister.cpp
        src/internal/ppu/registers/OamDataRegister.h
        src/internal/ppu/registers/OamDmaRegister.cpp
        src/internal/ppu/registers/OamDmaRegister.h
        src/internal/ppu/registers/PPURegister.cpp
        src/internal/ppu/registers/PPURegister.h
        src/internal/ppu/registers/ScrollRegister.cpp
        src/internal/ppu/registers/ScrollRegister.h
        src/internal/ppu/registers/StatusRegister.cpp
        src/internal/ppu/registers/StatusRegister.h
        src/internal/ppu/registers/AddrRegister.cpp
        src/internal/ppu/registers/AddrRegister.h
)

target_link_libraries(nes_emulator
    PRIVATE
        nes_core
        SDL2::SDL2
        SDL2::SDL2main
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)

    add_executable(cpu_tests
        test/cpu_tests.cc
            test/ppu_tests.cc
            #            src/internal/cpu_opcode_impl.cpp
#            src/internal/rom.cpp
#            src/internal/rom.h
#            src/internal/ppu/NesPPU.cpp
#            src/internal/ppu/NesPPU.h
#            src/internal/ppu/AddrRegister.cpp
#            src/internal/ppu/AddrRegister.h
#            src/internal/ppu/ControllerRegister.cpp
#            src/internal/ppu/ControllerRegister.h
#            src/internal/ppu/DataRegister.cpp
#            src/internal/ppu/DataRegister.h
#            src/internal/ppu/MaskRegister.cpp
#            src/internal/ppu/MaskRegister.h
#            src/internal/ppu/StatusRegister.cpp
#            src/internal/ppu/StatusRegister.h
#            src/internal/ppu/OamAddressRegister.cpp
#            src/internal/ppu/OamAddressRegister.h
#            src/internal/ppu/OamDataRegister.cpp
#            src/internal/ppu/OamDataRegister.h
#            src/internal/ppu/ScrollRegister.cpp
#            src/internal/ppu/ScrollRegister.h
#            src/internal/ppu/OamDmaRegister.cpp
#            src/internal/ppu/OamDmaRegister.h
#            src/internal/ppu/PPURegister.cpp
#            src/internal/ppu/PPURegister.h
#            src/internal/Color.cpp
#            src/internal/Color.h
#            src/internal/ppu/registers/ControllerRegister.cpp
#            src/internal/ppu/registers/ControllerRegister.h
#            src/internal/ppu/registers/DataRegister.cpp
#            src/internal/ppu/registers/DataRegister.h
#            src/internal/ppu/registers/MaskRegister.cpp
#            src/internal/ppu/registers/MaskRegister.h
#            src/internal/ppu/registers/OamAddressRegister.cpp
#            src/internal/ppu/registers/OamAddressRegister.h
#            src/internal/ppu/registers/OamDataRegister.cpp
#            src/internal/ppu/registers/OamDataRegister.h
#            src/internal/ppu/registers/OamDmaRegister.cpp
#            src/internal/ppu/registers/OamDmaRegister.h
#            src/internal/ppu/registers/PPURegister.cpp
#            src/internal/ppu/registers/PPURegister.h
#            src/internal/ppu/registers/ScrollRegister.cpp
#            src/internal/ppu/registers/ScrollRegister.h
#            src/internal/ppu/registers/StatusRegister.cpp
#            src/internal/ppu/registers/StatusRegister.h
#            src/internal/ppu/registers/AddrRegister.cpp
#            src/internal/ppu/registers/AddrRegister.h
    )

    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_INCLUDE_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
        target_compile_options(cpu_tests
            PRIVATE
                -iquote${VCPKG_INCLUDE_DIR}
                -I${VCPKG_INCLUDE_DIR}
        )
    endif()

    target_compile_definitions(cpu_tests
        PRIVATE
            PROJECT_SOURCE_DIR_PATH="${CMAKE_CURRENT_SOURCE_DIR}"
    )

    target_link_libraries(cpu_tests
        PRIVATE
            nes_core
            GTest::gtest_main
    )

    get_target_property(GTEST_MAIN_LOCATION GTest::gtest_main IMPORTED_LOCATION)
    if(NOT GTEST_MAIN_LOCATION)
        get_target_property(GTEST_MAIN_LOCATION GTest::gtest_main IMPORTED_LOCATION_RELEASE)
    endif()
    if(GTEST_MAIN_LOCATION)
        get_filename_component(GTEST_RUNTIME_DIR "${GTEST_MAIN_LOCATION}" DIRECTORY)
        set_target_properties(cpu_tests
            PROPERTIES
                BUILD_RPATH "${GTEST_RUNTIME_DIR}"
        )
    endif()

    add_test(NAME cpu_tests COMMAND cpu_tests)
endif()
