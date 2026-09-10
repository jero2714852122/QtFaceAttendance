add_library(face_attendance_common STATIC
    frameprotocol.h
    frameprotocol.cpp
)

target_link_libraries(face_attendance_common
    PUBLIC
        Qt6::Core
)

target_include_directories(face_attendance_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)