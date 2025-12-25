add_library(redis_lib 
    server.cpp 
    client.cpp
    utils.cpp
    connection.cpp
    buffer.cpp
    codec.cpp 
    encoder.cpp
    variable_encoder.cpp
    file_descriptor.cpp
    tagged_encoder.cpp
  )


target_include_directories(redis_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Redis redis.m.cpp)
target_link_libraries(Redis PRIVATE redis_lib)
