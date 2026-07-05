add_library(redis_lib 
    redis_client.cpp
    redis_server.cpp
    redis_schema.cpp
    redis_meta.cpp 
    redis_hashtable.cpp
  )


target_include_directories(redis_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(redis_lib PRIVATE net_lib spdlog::spdlog)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(redis_lib PRIVATE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
    )
else()
    target_compile_definitions(redis_lib PRIVATE
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO
    )
endif()


