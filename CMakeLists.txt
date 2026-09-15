cmake_minimum_required(VERSION 3.21)
project(LeafGreenPSX LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

psn00bsdk_add_executable(leafgreen_psx GPREL
    src/main.c
    src/platform/psx/platform_psx.c
)

target_include_directories(leafgreen_psx PRIVATE include)

psn00bsdk_add_cd_image(
    leafgreen_psx_cd
    leafgreen-psx
    iso.xml
    DEPENDS leafgreen_psx
)
