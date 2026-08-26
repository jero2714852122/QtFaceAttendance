qt_add_executable(face_attendance_server
    main.cpp
)

target_link_libraries(face_attendance_server
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        Qt6::Widgets
)

set_target_properties(face_attendance_server PROPERTIES
    WIN32_EXECUTABLE ON
)
