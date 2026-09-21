cmake_minimum_required(VERSION 3.15)
project(ChatServer VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(SOURCES
    main.cpp
    ChatServer.cpp
    DataBaseManage.cpp
    HttpRequest.cpp
    ThreadPool.cpp
)

set(HEADERS
    ChatServer.h
    DataBaseManage.h
    HttpRequest.h
    ThreadPool.h
    SafeSocket.h
)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
find_package(pqxx REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE pqxx::pqxx)
if(WIN32)
target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/MANIFESTDEPENDENCY:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
endif()
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
