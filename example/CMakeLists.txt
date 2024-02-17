
aux_source_directory(../src OtherSrc)

set(CMAKE_BUILD_TYPE Debug)

add_executable(Server ${OtherSrc} EchoServer.cpp)
target_link_libraries(Server pthread)
target_compile_options(Server PUBLIC -g)

add_executable(Client Client.cpp)
target_compile_options(Client PUBLIC -g)