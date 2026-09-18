qt_add_executable(face_attendance_server
    main.cpp

    network/server.h
    network/server.cpp
    network/clientconnection.h
    network/clientconnection.cpp

    ui/serverwindow.h
    ui/serverwindow.cpp
    controller/servercontroller.h controller/servercontroller.cpp
    database/databasemanager.h database/databasemanager.cpp
)

target_link_libraries(face_attendance_server
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        Qt6::Widgets
        face_attendance_common
)

target_include_directories(face_attendance_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(face_attendance_server PROPERTIES
    WIN32_EXECUTABLE ON
)