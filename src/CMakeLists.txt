aux_source_directory(. SRC_LIST)

add_executable(Server ${SRC_LIST} ../main.cpp)

target_link_libraries(Server pthread)

