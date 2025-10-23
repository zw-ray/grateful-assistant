# 开发者指南

本文档提供了Grateful Assistant项目的开发环境搭建、代码结构和开发流程等信息，帮助开发者快速上手并参与项目开发。

## 🚀 环境搭建

### 系统要求
- **操作系统**: Windows 10/11, macOS 10.15+, Ubuntu 20.04+
- **内存**: 至少 4GB RAM
- **磁盘空间**: 至少 10GB 可用空间

### 开发工具

#### 编译器
- **Windows**: Visual Studio 2019 或更高版本（安装C++开发工具）
- **macOS**: Xcode 12 或更高版本（安装Command Line Tools）
- **Linux**: GCC 9 或更高版本，或 Clang 10 或更高版本

#### 构建工具
- CMake 3.15 或更高版本
- Ninja（可选，推荐用于更快的构建速度）

#### IDE（推荐）
- Visual Studio（Windows）
- CLion（跨平台）
- Visual Studio Code（跨平台，配合C/C++扩展）

#### 版本控制
- Git

## 🏗️ 项目结构

Grateful Assistant 采用模块化的代码结构，主要分为以下几个部分：

```
gratefulassistant/
├── src/                    # 源代码目录
│   ├── core/               # 核心功能模块
│   ├── ui/                 # 用户界面相关代码
│   ├── todo/               # 待办事项功能模块
│   ├── ai/                 # AI助手功能模块
│   ├── utils/              # 通用工具类和函数
│   └── main.cpp            # 程序入口
├── include/                # 公共头文件
├── tests/                  # 单元测试和集成测试
├── assets/                 # 图像、音频等资源文件
├── docs/                   # 项目文档
├── build/                  # 构建目录（推荐，不包含在版本控制中）
├── CMakeLists.txt          # CMake主配置文件
├── README.md               # 项目说明文档
└── LICENSE                 # 许可证文件
```

### 核心模块说明

1. **core模块**
   - 应用程序核心逻辑
   - 配置管理
   - 日志系统
   - 数据库连接管理

2. **ui模块**
   - 用户界面实现
   - 事件处理
   - 界面组件

3. **todo模块**
   - 待办事项数据模型
   - 待办事项管理功能
   - 日历、天气、新闻等集成功能

4. **ai模块**
   - AI聊天功能
   - 图像生成接口
   - 语音识别和合成

5. **utils模块**
   - HTTP客户端
   - 日期时间处理
   - 字符串工具
   - 文件操作

## 🔧 编译指南

### Windows (Visual Studio)

```powershell
# 克隆仓库
git clone https://github.com/yourusername/gratefulassistant.git
cd gratefulassistant

# 创建构建目录
mkdir build && cd build

# 使用CMake配置项目
cmake .. -G "Visual Studio 17 2022" -A x64

# 打开解决方案
start GratefulAssistant.sln

# 或者直接使用CMake构建
cmake --build . --config Release
```

### macOS

```bash
# 克隆仓库
git clone https://github.com/yourusername/gratefulassistant.git
cd gratefulassistant

# 创建构建目录
mkdir build && cd build

# 使用CMake配置项目
cmake .. -G "Xcode"

# 打开Xcode项目
open GratefulAssistant.xcodeproj

# 或者直接使用CMake构建
cmake --build . --config Release
```

### Linux

```bash
# 克隆仓库
git clone https://github.com/yourusername/gratefulassistant.git
cd gratefulassistant

# 创建构建目录
mkdir build && cd build

# 使用CMake配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译项目
make -j$(nproc)
```

## 🧪 测试

项目包含单元测试和集成测试，使用[请添加实际使用的测试框架]进行测试。

```bash
# 在构建目录中运行测试
cd build
test/run_tests
```

## 📋 代码规范

为了保持代码的一致性和可读性，项目遵循以下代码规范：

- 采用4个空格进行缩进
- 花括号放在行尾
- 类名使用PascalCase命名法
- 函数和变量使用camelCase命名法
- 私有成员变量以下划线（_）开头
- 每行代码不超过100个字符
- 使用现代C++特性（C++17/20）
- 添加适当的注释，特别是公共接口

## 🚦 开发流程

1. **创建分支**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **开发功能**
   - 遵循代码规范
   - 为新功能添加测试
   - 确保所有测试通过

3. **提交代码**
   ```bash
   git add .
   git commit -m "描述你的更改"
   ```

4. **推送到远程仓库**
   ```bash
   git push origin feature/your-feature-name
   ```

5. **创建Pull Request**
   - 描述你的更改
   - 关联相关的Issue
   - 等待代码审查

## 📝 文档编写

- 为公共API添加详细的文档注释
- 更新README.md和其他相关文档
- 为新功能编写使用指南

## ❓ 常见问题

### 编译失败怎么办？
- 检查是否安装了所有依赖项
- 确认编译器版本是否符合要求
- 检查CMake配置是否正确
- 查看详细的错误信息并尝试解决

### 如何添加新功能？
- 首先创建一个Issue描述你的功能
- 创建一个新的分支进行开发
- 遵循代码规范和开发流程
- 添加测试并确保通过
- 提交Pull Request

### 如何报告Bug？
- 在GitHub上创建一个新的Issue
- 描述Bug的详细信息
- 提供复现步骤
- 如果可能，附上截图或错误日志

## 📞 联系我们

如果您有任何问题或建议，请通过以下方式联系我们：
- GitHub Issues: [https://github.com/yourusername/gratefulassistant/issues](https://github.com/yourusername/gratefulassistant/issues)