# ⚙️ 高级配置指南（v0.1.0）

本指南介绍构建选项、依赖管理与环境变量配置，确保在多平台稳定构建与运行。

## 构建选项（CMake Options）

文件：`cmake/options.cmake`

```cmake
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_TESTING "Enable testing" ON)

option(BUILD_FILE_SERVICE "Build file service" OFF)
option(BUILD_GAME_SERVICE "Build game service" OFF)
option(BUILD_MUSIC_SERVICE "Build music service" OFF)
option(BUILD_NOVEL_SERVICE "Build novel service" OFF)
option(BUILD_VIDEO_SERVICE "Build video service" OFF)
option(BUILD_WEB_SERVICE "Build web service" OFF)
option(BUILD_WRITER_SERVICE "Build writer service" OFF)

option(BUILD_AI_PROVIDERS "Build AI providers extension" ON)
option(BUILD_WEATHER_PROVIDERS "Build weather providers extension" OFF)
```

使用示例：

```bash
# Release 构建，启用共享库与 AI 扩展
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_AI_PROVIDERS=ON
cmake --build build -j
```

调试构建会在顶层 `CMakeLists.txt` 中定义 `GA_DEBUG` 宏：

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(GA_DEBUG)
endif()
```

## 依赖管理（vcpkg 可选）

文件：`vcpkg.json` 包含依赖：`fmt`、`nlohmann-json`、`spdlog`、`sqlite3`、`sqlite-orm`、`portaudio`、`poco`、`openssl`、`cpp-httplib` 等。

使用方式：

```bash
# 设置 vcpkg 工具链
export VCPKG_ROOT=~/vcpkg
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Qt6 安装与环境

- 需要 `Qt6 Widgets/Core`。
- 若 CMake 报错 `Qt6 not found.`，请设置 `Qt6_DIR` 指向 Qt6 安装的 CMake 模块目录：

```bash
# 常见安装路径示例（Linux）
export Qt6_DIR=/opt/Qt/6.6.0/gcc_64/lib/cmake/Qt6
```

验证：第一次配置时 CMake 会输出 `Qt6 found.`，并显示 `Qt6Widgets_INCLUDE_DIRS` 与 `Qt6Core_INCLUDE_DIRS`。

Windows 部署可用 `windeployqt`（在 `source/gui/CMakeLists.txt` 已集成 POST_BUILD）。

## 输出目录与运行方式

- 可执行文件：`build/bin/grateassistant`
- 动态库与资源：`build/bin/`
- 静态库：`build/lib/`

示例：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/bin/grateassistant
```

## 资源与样式

- 资源文件聚合：`resource/res.qrc`
- 样式：`resource/style/dark.qss`、`resource/style/light.qss`
- 字体与图标位于 `resource/fonts/` 与 `resource/icons/`

在 `source/gui/CMakeLists.txt` 中已启用：

```cmake
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
```

## 术语对照（中/英）

- Provider（提供者）：封装外部服务（AI/天气）
- Model（模型）：Provider 下的具体模型配置
- Service（服务）：功能模块（todo、weather、audio 等）
- Event Bus（事件总线）：跨模块消息分发机制

## 故障定位要点

- Qt6 未找到：检查 `Qt6_DIR`；确认安装了 Widgets/Core 组件。
- 依赖缺失：使用 vcpkg 或系统包管理器安装。
- 运行库缺失（Windows）：`windeployqt` 复制依赖到 `bin`。
- 数据库文件：示例资源 `resource/db/cities_code.db` 已打包至资源（无需磁盘路径）。