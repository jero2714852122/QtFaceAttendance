qt_add_executable(face_attendance_client
    main.cpp
)

target_link_libraries(face_attendance_client
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
)

set_target_properties(face_attendance_client PROPERTIES
    WIN32_EXECUTABLE ON
)
