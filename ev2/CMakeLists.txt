cmake_minimum_required(VERSION 3.15)

project(meltdown VERSION 0.0.2 LANGUAGES CXX)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h src/*.hpp)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS src/*.h src/*.hpp)

add_library(meltdown STATIC ${sources} ${headers})

set_target_properties(meltdown PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(meltdown
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    # $<INSTALL_INTERFACE:include>
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(meltdown PUBLIC EV_DEBUG)

target_precompile_headers(meltdown
PRIVATE
    "src/evpch.hpp"
)

target_link_libraries(meltdown PUBLIC glm glad glfw3 tinyobjloader tinygltfloader reactphysics3d nlohmann)
target_link_libraries(meltdown PRIVATE leb cbt spdlog)

target_include_directories(meltdown PUBLIC ${OPENGL_INCLUDE_DIRS})
target_link_libraries(meltdown PRIVATE ${OPENGL_LIBRARY})


if(WIN32)
else()
target_link_libraries(meltdown PUBLIC dl X11 pthread)
endif()

target_compile_options(meltdown PRIVATE -Wall -Wno-unused-function -Werror=return-type)

message(STATUS "GLFW3 include ${glfw3}")
message(STATUS "GLFW3 link ${GLFW3_LIBRARY}")

message(STATUS "OPENGL include ${OPENGL_INCLUDE_DIRS}")
message(STATUS "OPENGL link ${OPENGL_LIBRARY}")