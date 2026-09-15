cmake_minimum_required(VERSION 3.21)
project(leafgreen_psx LANGUAGES C ASM)

set(LGPSX_SOURCES
    src/main.c
    src/platform/psx/platform_psx.c
    src/platform/psx/text_psx.c
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

option(LGPSX_FONT_DEMO "Embed a locally generated LeafGreen font demo" OFF)
set(LGPSX_FONT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated/font" CACHE PATH "Local font extraction output")
if(LGPSX_FONT_DEMO)
    if(NOT EXISTS "${LGPSX_FONT_DIR}/font_data.h")
        message(FATAL_ERROR "Run tools/font/extract_font.py with your verified ROM before enabling LGPSX_FONT_DEMO")
    endif()
    target_include_directories(leafgreen_psx PRIVATE "${LGPSX_FONT_DIR}")
    target_compile_definitions(leafgreen_psx PRIVATE LGPSX_FONT_DEMO=1)
endif()
