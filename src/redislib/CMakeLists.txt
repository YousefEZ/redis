add_library(redis_lib 
    server.cpp 
    client.cpp
)


target_include_directories(redis_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(redlis_lib PRIVATE net_lib)


