set(TARGET_NAME websocket)

set(${TARGET_NAME}_HEADERS
    Threading.h
    WebSocket.h
)

set(${TARGET_NAME}_SOURCES
    WebSocket.cpp
)

add_library(${TARGET_NAME}
    ${${TARGET_NAME}_SOURCES}
    ${${TARGET_NAME}_HEADERS}
)
add_library(ad::${TARGET_NAME} ALIAS ${TARGET_NAME})

cmc_target_current_include_directory(${TARGET_NAME})

set_target_properties(${TARGET_NAME} PROPERTIES
                      VERSION "${${PROJECT_NAME}_VERSION}"
)


##
## Dependencies
##

cmc_find_dependencies()

target_link_libraries(${TARGET_NAME}
    PUBLIC
        Boost::boost
    PRIVATE
        $<$<PLATFORM_ID:Linux>:Threads::Threads>
)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    # On Windows, header-only asio tries to link against date_time
    # see: https://www.boost.org/doc/libs/1_71_0/doc/html/boost_asio/using.html#boost_asio.using.dependencies
    target_compile_definitions(${TARGET_NAME}
        PUBLIC
            BOOST_DATE_TIME_NO_LIB
    )
endif()


##
## Install
##

install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME}Targets)

include(cmc-install)
# Install the headers preserving the directory hierarchy
cmc_install_with_folders(FILES ${${TARGET_NAME}_HEADERS}
                         DESTINATION include/${TARGET_NAME}/${TARGET_NAME}
)
cmc_install_packageconfig(${TARGET_NAME} ${TARGET_NAME}Targets ${PROJECT_NAME}
                          FIND_FILE CMakeFinds.cmake.in
                          NAMESPACE ad::
)
