# 注意
这个项目的画布部分是由SDL3的mingw库实现的,可到github上下载.一定要在cmake里把SDL的引用路径改成自己文件的路径,不然就要报一堆错了.

实现的项目为弹幕设射击游戏。 实验对应的main函数在/gui/main_paintboard.cpp中，项目的在/game/main.cpp中，主文件夹下的test仅用于开发过程中的调试，请自行调整CMakeList实现项目的切换。

项目基于C++20进行开发，例如<numbers>中的数学常数等，请确保编译器的版本支持该标准，可以自行降级，但请尽量不要低于C++17标准，因为项目中大量使用variant。不要用clion自带的mingw，用不了，请自行配置工具链
