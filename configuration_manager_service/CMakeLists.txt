cmake_minimum_required(VERSION 3.10)
project(configuration_manager_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core DBus REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    ConfigurationManager.cpp
    ConfigurationManager.hpp
)

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::DBus)