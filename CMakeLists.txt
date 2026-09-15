cmake_minimum_required(VERSION 3.20)
project(leafgreen_psx C)

add_executable(leafgreen_psx
    src/main.c
    src/platform/psx/platform_psx.c
    src/game/text.c
    src/game/trade_record.c
    src/overworld/overworld.c
    src/script/vm.c
)
target_include_directories(leafgreen_psx PRIVATE include)
target_compile_features(leafgreen_psx PRIVATE c_std_11)
target_compile_options(leafgreen_psx PRIVATE -Wall -Wextra)

# PSn00bSDK supplies these helpers when its toolchain is active.
# Keep generated proprietary assets out of the executable until the local
# extraction pipeline has produced them; M0 host logic can compile without them.
if(COMMAND psn00bsdk_add_executable)
    psn00bsdk_add_executable(leafgreen_psx GPREL)
endif()
if(COMMAND psn00bsdk_add_cd_image)
    psn00bsdk_add_cd_image(leafgreen_psx_iso leafgreen_psx iso.xml DEPENDS leafgreen_psx)
endif()
