cmake_minimum_required(VERSION 3.21)
project(leafgreen_psx LANGUAGES C ASM)

set(LGPSX_SOURCES
    src/main.c
    src/platform/psx/platform_psx.c
    src/game/text.c
    src/game/trade_record.c
    src/game/save.c
    src/game/resource.c
    src/overworld/overworld.c
    src/script/vm.c
)

# PSn00bSDK's toolchain defines psn00bsdk_add_executable(). Do not create the
# target with add_executable() first: the SDK helper owns executable creation
# and PS-EXE conversion/link settings.
if(NOT COMMAND psn00bsdk_add_executable)
    message(FATAL_ERROR "PSn00bSDK toolchain is required. Configure with CMakePresets.json or set CMAKE_TOOLCHAIN_FILE to PSn00bSDK's sdk.cmake.")
endif()

psn00bsdk_add_executable(leafgreen_psx GPREL ${LGPSX_SOURCES})
target_include_directories(leafgreen_psx PRIVATE include)
target_compile_features(leafgreen_psx PRIVATE c_std_11)
target_compile_options(leafgreen_psx PRIVATE -Wall -Wextra)

psn00bsdk_add_cd_image(
    leafgreen_psx_iso
    leafgreen_psx
    iso.xml
    DEPENDS leafgreen_psx
)
