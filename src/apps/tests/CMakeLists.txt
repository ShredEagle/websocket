string(TOLOWER ${PROJECT_NAME} _lower_project_name)
set(TARGET_NAME ${_lower_project_name}_tests)

set(${TARGET_NAME}_HEADERS
    catch.hpp
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
