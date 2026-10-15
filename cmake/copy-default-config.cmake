# 把默认 config.ini 放到目标目录，但只在目标目录里还没有这个文件时才放。
#
# 为什么不用 copy_if_different：它在"内容不一样"的时候会覆盖。
# 而用户很可能已经把可执行文件旁边的 config.ini 改成自己那台机器的 IP，
# 每编译一次就冲掉一次真实配置，是不能接受的。
#
# 用法（由 apps/*/CMakeLists.txt 的 POST_BUILD 调用）：
#   cmake -DSOURCE_CONFIG=<默认配置> -DTARGET_DIR=<可执行文件目录>
#         -P cmake/copy-default-config.cmake

if(NOT DEFINED SOURCE_CONFIG OR NOT DEFINED TARGET_DIR)
    message(FATAL_ERROR "SOURCE_CONFIG 和 TARGET_DIR 都必须提供")
endif()

# Windows 传进来的路径可能带反斜杠，统一成正斜杠，
# 否则 "${TARGET_DIR}/config.ini" 里的 \c 之类会被当成转义序列。
file(TO_CMAKE_PATH "${TARGET_DIR}" target_dir)

set(destination "${target_dir}/config.ini")

if(EXISTS "${destination}")
    message(STATUS "已存在 config.ini，保留不动：${destination}")
else()
    file(COPY "${SOURCE_CONFIG}" DESTINATION "${target_dir}")
    message(STATUS "已放置默认 config.ini：${destination}")
endif()
