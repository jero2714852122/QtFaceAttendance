find_package(OpenCV 4.12 REQUIRED COMPONENTS core imgproc videoio)

qt_add_executable(face_attendance_client
    main.cpp
    ui/mainwindow.h ui/mainwindow.cpp
    camera/cameracontroller.h camera/cameracontroller.cpp
    vision/facedetector.h vision/facedetector.cpp
    network/networkclient.h network/networkclient.cpp
    vision/frameprocessor.h vision/frameprocessor.cpp
    controller/attendancecontroller.h controller/attendancecontroller.cpp
)

target_link_libraries(face_attendance_client
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
        ${OpenCV_LIBS}
        face_attendance_common
)

target_include_directories(face_attendance_client
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(TARGET face_attendance_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:opencv_world>
        $<TARGET_FILE_DIR:face_attendance_client>
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:face_attendance_client>/models
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/assets/haarcascade_frontalface_default.xml
        $<TARGET_FILE_DIR:face_attendance_client>/models/haarcascade_frontalface_default.xml
)

set_target_properties(face_attendance_client PROPERTIES
    WIN32_EXECUTABLE ON
)
