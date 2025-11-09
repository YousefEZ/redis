add_executable(Redis)


target_sources(Redis PRIVATE 
    redis.m.cpp
    server.cpp 
    client.cpp
    utils.cpp
  )
