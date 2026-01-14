add_subdirectory("targets")
add_executable(tests test.cpp)

target_link_libraries(tests PRIVATE sdb::libsdb Catch2::Catch2WithMain)

