find_package(Qt6 6.9 REQUIRED COMPONENTS Test Sql)

# 协议测试只依赖 common：不连网络、不碰界面，跑起来是毫秒级的。
add_executable(tst_frameprotocol
    tst_frameprotocol.cpp
)

target_link_libraries(tst_frameprotocol
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

# 测试程序得在自己的目录里找到 Qt 的 DLL，否则 ctest 一跑就是
# "找不到 Qt6Core.dll"。SQLite 驱动属于插件，Qt 会按安装目录自己找。
foreach(test_target tst_frameprotocol tst_database)
    set_target_properties(${test_target} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )

    add_custom_command(TARGET ${test_target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Core>
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
