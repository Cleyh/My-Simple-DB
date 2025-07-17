cmake_minimum_required(VERSION 3.21)
project(my-simple-db VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS asio)

# 自动收集所有源码
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h)
add_executable(my-simple-db ${SOURCES})

# 链接依赖
target_link_libraries(my-simple-db
    PRIVATE
        nlohmann_json::nlohmann_json
        Boost::asio
)