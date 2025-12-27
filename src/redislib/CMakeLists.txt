add_library(redis_lib 
    redis_client.cpp
    redis_server.cpp
)


target_include_directories(redis_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(redis_lib PRIVATE net_lib)


