# 把默认配置模板放到可执行文件旁边，改名成 config.ini，
# 只在目标目录里还没有 config.ini 时才放。
#
# 为什么不用 file(COPY)：
#   它只把文件复制进目标目录，**保留原来的文件名**。模板叫 config.ini.default，
#   复制过去还是叫 config.ini.default，程序按 config.ini 去找就会找不到。
#   configure_file 允许显式指定输出文件名，正好补上这一点。
#
# 为什么不用 copy_if_different：
#   它在"内容不一样"的时候会覆盖。而用户很可能已经把可执行文件旁边的
#   config.ini 改成自己那台机器的 IP，每编译一次就冲掉一次真实配置，
#   是不能接受的。
#
# 用法（由 apps/*/CMakeLists.txt 的 POST_BUILD 调用）：
#   cmake -DSOURCE_CONFIG=<默认配置模板> -DTARGET_DIR=<可执行文件目录>
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
    # COPYONLY 的意思是"逐字节照抄，不展开里面的 ${} 变量"。
    # 这一点很重要：配置文件里出现 ${...} 时不该被 CMake 当成变量吞掉。
    configure_file("${SOURCE_CONFIG}" "${destination}" COPYONLY)
    message(STATUS "已放置默认 config.ini：${destination}")
endif()
