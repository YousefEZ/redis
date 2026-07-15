add_subdirectory(net)
add_subdirectory(redislib)

add_executable(Server server.m.cpp)
target_link_libraries(Server PRIVATE redis_lib net_lib spdlog::spdlog)

add_executable(Client client.m.cpp)
target_link_libraries(Client PRIVATE redis_lib net_lib spdlog::spdlog)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Server PRIVATE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
    )
    target_compile_definitions(Client PRIVATE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
    )
else()
    target_compile_definitions(Server PRIVATE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
    )
    target_compile_definitions(Client PRIVATE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
    )
endif()
