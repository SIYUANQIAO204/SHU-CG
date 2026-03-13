cmake_minimum_required(VERSION 3.22)
project(GraphicCore)  # 重命名项目

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 核心源文件（包含core目录下的实现和test.cpp）
file(GLOB CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp")
file(GLOB GUI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/gui/*.cpp")
file(GLOB_RECURSE GAME_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/game/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/game/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/game/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/game/**/*.h"
)
set(SOURCES ${CORE_SOURCES})

# 2. 头文件目录
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/gui")

# 3. 找到本机 SDL3

#    3.2 手动指定头文件/库目录（vcpkg 用户可删掉这两行）(注意这里我安装的是SDL3的mingw库,用了绝对路径,你们在运行的时候要修改成自己的路径)
set(SDL3_INCLUDE_DIRS1 "D:/my application/SDL3-devel-3.2.24-mingw/SDL3-3.2.24/x86_64-w64-mingw32/include/SDL3")
set(SDL3_INCLUDE_DIRS2 "D:/my application/SDL3-devel-3.2.24-mingw/SDL3-3.2.24/x86_64-w64-mingw32/include")
set(SDL3_LIBRARIES
    "D:/my application/SDL3-devel-3.2.24-mingw/SDL3-3.2.24/x86_64-w64-mingw32/lib/libSDL3.dll.a"
    "D:/my application/SDL3-devel-3.2.24-mingw/SDL3-3.2.24/x86_64-w64-mingw32/lib/libSDL3_test.a")

# 4. 生成可执行程序
add_executable(GraphicCore ${SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/test.cpp"
        game/GameObject/TrajectoryController.h)
target_compile_definitions(GraphicCore PRIVATE RENDERENGINE_DEBUG)

# 5. 生成「交互画板」可执行文件（零修改核心）
add_executable(PaintBoard ${SOURCES} ${GUI_SOURCES})
target_include_directories(PaintBoard PRIVATE ${SDL3_INCLUDE_DIRS1} ${SDL3_INCLUDE_DIRS2})
target_link_libraries(PaintBoard PRIVATE ${SDL3_LIBRARIES})
target_compile_definitions(PaintBoard PRIVATE RENDERENGINE_DEBUG)

# 创建游戏可执行文件
add_executable(STGGame 
    ${CORE_SOURCES}  # 引擎核心
    ${GAME_SOURCES}
        game/GameObject/TrajectoryController.h
        game/GameObject/TrajectoryController.cpp
        # 游戏代码
)

# 链接库
target_link_libraries(STGGame PRIVATE ${SDL3_LIBRARIES})
target_include_directories(STGGame PRIVATE 
    ${SDL3_INCLUDE_DIRS1} 
    ${SDL3_INCLUDE_DIRS2}
    "${CMAKE_CURRENT_SOURCE_DIR}/game"  # 添加game目录
)

# 拷贝SDL3.dll
add_custom_command(TARGET STGGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "D:/my application/SDL3-devel-3.2.24-mingw/SDL3-3.2.24/x86_64-w64-mingw32/bin/SDL3.dll"
    $<TARGET_FILE_DIR:STGGame>)

# 编译选项
target_compile_definitions(STGGame PRIVATE RENDERENGINE_DEBUG)

# 6. 把 SDL3.dll 自动拷到输出目录（省得手抖）(注意这里我安装的是SDL3的mingw库,用了绝对路径,你们在运行的时候也要修改成自己的路径)
add_custom_command(TARGET PaintBoard POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "D:/my application/SDL3-devel-3.2.24-mingw/SDL3-3.2.24/x86_64-w64-mingw32/bin/SDL3.dll"
    $<TARGET_FILE_DIR:PaintBoard>)

#编译选项（保留原优化配置）
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(GraphicCore PRIVATE -O3 -flto -march=native)
        target_link_options(GraphicCore PRIVATE -flto -O3)
    elseif (MSVC)
        target_compile_options(GraphicCore PRIVATE /O2 /GL)
        target_link_options(GraphicCore PRIVATE /LTCG)
    endif ()
endif ()