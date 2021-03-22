cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(CppThreadPool)

find_package(Threads)
#set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-lpthread" LINK_FLAGS "-lpthread")
add_executable(
    ${PROJECT_NAME}
    main.cpp
    Cpp11ThreadPool.hpp
    SyncQueue.hpp
    ThreadPool.hpp)


target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})