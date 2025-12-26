add_subdirectory(net)
add_subdirectory(redislib)

add_executable(Redis redis.m.cpp)
target_link_libraries(Redis PRIVATE redis_lib net_lib)
