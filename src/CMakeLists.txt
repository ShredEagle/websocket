add_subdirectory(libs/websocket/websocket)

option(BUILD_tests "Build the test applications" ON)
if(BUILD_tests)
    add_subdirectory(apps/tests)
    add_subdirectory(apps/EchoServer)
endif()
