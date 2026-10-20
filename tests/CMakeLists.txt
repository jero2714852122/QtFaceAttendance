find_package(Qt6 6.9 REQUIRED COMPONENTS Test Sql)

# 协议测试只依赖 common：不连网络、不碰界面，跑起来是毫秒级的。
add_executable(tst_frameprotocol
    tst_frameprotocol.cpp
)
add_executable(tst_appconfig
    tst_appconfig.cpp
)

target_link_libraries(tst_frameprotocol
    PRIVATE
        Qt6::Test
        face_attendance_common
)
target_link_libraries(tst_appconfig
    PRIVATE
        Qt6::Test
        face_attendance_common
)

# 数据库测试直接编译仓储层的源文件。目标多起来之后再抽成静态库也不迟，
# 现在为了两个测试文件去动应用的构建结构不划算。
add_executable(tst_database
    tst_database.cpp
    ${CMAKE_SOURCE_DIR}/apps/server/database/databasemanager.cpp
    ${CMAKE_SOURCE_DIR}/apps/server/database/employeerepository.cpp
    ${CMAKE_SOURCE_DIR}/apps/server/database/facetemplaterepository.cpp
    ${CMAKE_SOURCE_DIR}/apps/server/database/attendancerepository.cpp
)

target_include_directories(tst_database
    PRIVATE
        ${CMAKE_SOURCE_DIR}/apps/server
)

target_link_libraries(tst_database
    PRIVATE
        Qt6::Test
        Qt6::Sql
)

# 视觉测试要真的链接 OpenCV，所以单独找一次包。
# 只测"模型缺失"和"特征序列化"，不加载真模型，所以不需要 assets 里的 onnx。
find_package(OpenCV 4.12 REQUIRED COMPONENTS core imgproc objdetect dnn)

add_executable(tst_vision
    tst_vision.cpp
    ${CMAKE_SOURCE_DIR}/apps/server/vision/faceengine.cpp
)

target_include_directories(tst_vision
    PRIVATE
        ${CMAKE_SOURCE_DIR}/apps/server
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(tst_vision
    PRIVATE
        Qt6::Test
        Qt6::Core
        ${OpenCV_LIBS}
)

# OpenCV 的 DLL 也得放在测试程序旁边，否则一跑就是找不到 DLL。
add_custom_command(TARGET tst_vision POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:opencv_world>
        $<TARGET_FILE_DIR:tst_vision>
)

# 测试程序得在自己的目录里找到 Qt 的 DLL，否则 ctest 一跑就是
# "找不到 Qt6Core.dll"。SQLite 驱动是插件，要放在 exe 旁边的 sqldrivers 里。
foreach(test_target tst_frameprotocol tst_appconfig tst_database tst_vision)
    set_target_properties(${test_target} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )

    add_custom_command(TARGET ${test_target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Core>
            $<TARGET_FILE_DIR:${test_target}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Network>
            $<TARGET_FILE_DIR:${test_target}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Test>
            $<TARGET_FILE_DIR:${test_target}>
    )
endforeach()


add_custom_command(TARGET tst_database POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Sql>
        $<TARGET_FILE_DIR:tst_database>
)

add_test(NAME frameprotocol COMMAND tst_frameprotocol)
add_test(NAME database COMMAND tst_database)
add_test(NAME appconfig COMMAND tst_appconfig)
add_test(NAME vision COMMAND tst_vision)
