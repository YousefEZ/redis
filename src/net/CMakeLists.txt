add_library(net_lib 
    server.cpp 
    client.cpp
    utils.cpp
    connection.cpp
    buffer.cpp
    codec.cpp 
    encoder.cpp
    single_type_encoder.cpp
    file_descriptor.cpp
    tagged_encoder.cpp
  )


target_include_directories(net_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
