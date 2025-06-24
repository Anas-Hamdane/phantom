cmake_minimum_required(VERSION 3.15)
project(phantom)

set(CMAKE_CXX_STANDARD 17)
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "src/lexer/*.cpp"
     "src/parser/*.cpp" "src/phantom.cpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
