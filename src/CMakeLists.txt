add_subdirectory(net)
add_subdirectory(redislib)

add_executable(Server server.m.cpp)
target_link_libraries(Server PRIVATE redis_lib net_lib)

add_executable(Client client.m.cpp)
target_link_libraries(Client PRIVATE redis_lib net_lib)
