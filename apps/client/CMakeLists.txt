find_package(OpenCV 4.12 REQUIRED COMPONENTS core imgproc videoio)

qt_add_executable(face_attendance_client
    main.cpp
)

target_link_libraries(face_attendance_client
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
        ${OpenCV_LIBS}
)

target_include_directories(face_attendance_client
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

add_custom_command(TARGET face_attendance_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:opencv_world>
        $<TARGET_FILE_DIR:face_attendance_client>
)

set_target_properties(face_attendance_client PROPERTIES
    WIN32_EXECUTABLE ON
)
