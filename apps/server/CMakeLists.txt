find_package(OpenCV 4.12 REQUIRED COMPONENTS core imgproc objdetect dnn)
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
    database/employee.h
    database/employeerepository.h database/employeerepository.cpp
    vision/faceengine.h vision/faceengine.cpp
    database/facetemplate.h
    database/facetemplaterepository.h database/facetemplaterepository.cpp
)

target_link_libraries(face_attendance_server
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        Qt6::Widgets
        face_attendance_common
        ${OpenCV_LIBS}
)

target_include_directories(face_attendance_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OpenCV_INCLUDE_DIRS})
add_custom_command(TARGET face_attendance_server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:opencv_world>
            $<TARGET_FILE_DIR:face_attendance_server>
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:face_attendance_server>/models
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/assets/face_detection_yunet_2023mar.onnx
            $<TARGET_FILE_DIR:face_attendance_server>/models/face_detection_yunet_2023mar.onnx
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/assets/face_recognition_sface_2021dec.onnx
            $<TARGET_FILE_DIR:face_attendance_server>/models/face_recognition_sface_2021dec.onnx
    )
set_target_properties(face_attendance_server PROPERTIES
    WIN32_EXECUTABLE ON
)