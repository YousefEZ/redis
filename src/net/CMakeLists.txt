add_library(net_lib 
    net_server.cpp 
    net_utils.cpp
    net_connection.cpp
    net_buffer.cpp
    net_codec.cpp 
    net_encoder.cpp
    net_single_type_encoder.cpp
    net_file_descriptor.cpp
    net_tagged_encoder.cpp
  )


target_include_directories(net_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
