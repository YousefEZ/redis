add_library(redis_lib 
    server.cpp 
    client.cpp
    utils.cpp
    message_parsing.cpp 
    connection.cpp
    buffer.cpp
  )


target_include_directories(redis_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Redis redis.m.cpp)
target_link_libraries(Redis PRIVATE redis_lib)
