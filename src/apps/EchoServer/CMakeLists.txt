set(TARGET_NAME EchoServer)

set(${TARGET_NAME}_HEADERS
)

set(${TARGET_NAME}_SOURCES
)

add_executable(${TARGET_NAME}
    main.cpp
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS})


target_link_libraries(${TARGET_NAME}
    PRIVATE
        ad::websocket
)

set_target_properties(${TARGET_NAME} PROPERTIES
                      VERSION "${${PROJECT_NAME}_VERSION}"
)


##
## Install
##

install(TARGETS ${TARGET_NAME})
