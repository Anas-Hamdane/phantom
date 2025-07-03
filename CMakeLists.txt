cmake_minimum_required(VERSION 3.20)
project(phantom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
    "./src/Lexer/*.cpp"
    "./src/Parser/*.cpp"
    "./src/Data/*.cpp"
    "./src/LLVMVisitor/*.cpp"
    "./src/*.cpp"
)
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST)
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} LLVM-20)

# I don't know why this doesn't work FOR ME
# llvm_map_components_to_libnames(llvm_libs support core irreader)
# target_link_libraries(${PROJECT_NAME} ${llvm_libs})
