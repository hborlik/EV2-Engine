file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE include "src/*.h" "src/*.hpp")
add_executable(test_application ${sources} ${include})

target_include_directories(test_application PRIVATE
    "include"
)

if(WIN32)
    set(CMAKE_CXX_FLAGS "-static-libgcc -static-libstdc++")
endif()

target_link_libraries(test_application 
    meltdown
    ptree
)

set_target_properties(test_application PROPERTIES 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# still need to install assets!

# copy assets to build directory
# add_custom_command(
#     TARGET test_application PRE_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_CURRENT_SOURCE_DIR}/asset $<TARGET_FILE_DIR:test_application>/asset
# )

# still need to install shaders!
# copy shaders to build directory
# add_custom_command(
#     TARGET test_application PRE_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:test_application>/shaders
# )
