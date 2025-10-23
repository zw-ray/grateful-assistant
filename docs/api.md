# API 文档（v0.1.0）

- 模块索引：Kernel、GUI、AI、DB、Service。
- 术语对照：Provider（提供者）、Model（模型）、Service（服务）、Event（事件）。
- 代码一致性：本文档以 `include/` 与 `source/` 中实际类与命名保持一致。

## Kernel 模块

### Configuration

头文件：`include/kernel/Configuration.h`

```cpp
class Configuration {
public:
    static Configuration& getInstance();
    bool load(const std::string& configPath);
    bool save(const std::string& configPath);

    // JSON 路径访问
    std::string get(const std::string& key, const std::string& def = "") const;
    int getInt(const std::string& key, int def = 0) const;
    bool getBool(const std::string& key, bool def = false) const;

    void set(const std::string& key, const std::string& value);
    void set(const std::string& key, int value);
    void set(const std::string& key, bool value);

    // 数组与对象
    uint32_t arraySize(const std::string& base);
    void push_back(const std::string& base);
};
```

示例：写入 Provider 注册信息（见 AI ProviderManager）。

```cpp
auto &config = Configuration::getInstance();
config.push_back("/ai/providers");
config.set("/ai/providers/0/name", "siliconflow");
config.set("/ai/providers/0/base_url", "https://api.siliconflow.cn");
```

### EventBus / Events

头文件：`include/kernel/EventBus.h`, `include/kernel/Events.h`

- 用于模块间解耦通信，支持订阅 `Subscription` 与事件发布。
- GUI `MainWindow` 通过事件处理系统消息、唤醒词与录音完成事件。

```cpp
// 订阅与触发（示意）
Subscription s = EventBus::subscribe<SystemEvents::SystemMessageEvent>(handler);
EventBus::publish(SystemEvents::SystemMessageEvent{"hello"});
```

### Logger

头文件：`include/kernel/Logger.h`

- 日志级别：Debug/Info/Warn/Error。
- 用法：`Logger::logDebug("message")`。

## GUI 模块

### MainWindow

头文件：`include/gui/MainWindow.h`，源：`source/gui/MainWindow.cpp`

- 左侧：时间卡片与天气卡片；右侧：待办列表与输入区域。
- 浮动消息（`FloatMessage`）展示系统提示，语音录音层按唤醒词显示/隐藏。

```cpp
MainWindow w; w.show();
```

### TodoList / TodoInput / WeatherCard / DateTimeCard

- 组件来源：`source/gui/*.cpp` 与 `include/gui/*.h`。
- 资源：`resource/res.qrc`，样式：`resource/style/*.qss`。

## AI 模块

### ProviderManager

头文件：`include/ai/ProviderManager.h`，源：`source/ai/ProviderManager.cpp`

- 注册/反注册 Provider；将 Provider 与 Model 参数序列化到配置：

```cpp
auto &config = Configuration::getInstance();
config.push_back("/ai/providers");
const std::string baseKey = fmt::format("/ai/providers/{}", idx);
config.set(fmt::format("{}/name", baseKey), provider->getName());
// 写入 models 参数（max_tokens, temperature, top_p, top_k, repetition_penalty, enable_thinking, thinking_budget, enable_streaming）
```

### Model / Provider / RoleManager / IntentManager

- 作用：模型参数定义、Provider 封装、角色管理与意图管理。
- 参考：`include/ai/*` 与 `source/ai/*`。

## DB 模块

### DatabaseManager / TodoConnection / SiliconFlowModelInfoConnection

- 位置：`include/db/*`, `source/db/*`。
- 作用：数据库连接与具体表操作（如 Todo、模型信息）。
- 依赖：`sqlite3`、`sqlite-orm`，资源数据库示例：`resource/db/cities_code.db`。

## Service 模块

- 目录：`source/service/*`（todo、weather、audio、sync 等）。
- 统一实现接口：`include/kernel/IService.h` 与 `source/kernel/IService.cpp`。

## 常见模式与示例

### Qt6 与 GUI 链接

```cmake
find_package(Qt6 REQUIRED COMPONENTS Widgets)
target_link_libraries(grateassistant PRIVATE Qt6::Widgets gui db ai)
```

### 配置选项（CMake）

```cmake
# cmake/options.cmake
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_TESTING "Enable testing" ON)
option(BUILD_AI_PROVIDERS "Build AI providers extension" ON)
option(BUILD_WEATHER_PROVIDERS "Build weather providers extension" OFF)
```

## 错误处理

- 统一通过 `Logger` 输出错误信息；关键失败点使用 `message(FATAL_ERROR ...)` 或返回值校验。
- GUI 中通过 `FloatMessage` 显示系统消息队列，避免阻塞主线程。

## AI助手模块API

### AIAssistant类

**头文件**: `include/ai/AIAssistant.h`

```cpp
class AIAssistant {
public:
    // 单例模式获取实例
    static AIAssistant& getInstance();
    
    // 设置API密钥
    void setApiKey(const std::string& apiKey);
    
    // 聊天功能
    std::string chat(const std::string& message, const std::vector<std::string>& chatHistory = {});
    
    // 生成图像
    std::string generateImage(const std::string& prompt, int width = 512, int height = 512);
    
    // 语音识别
    std::string recognizeSpeech(const std::vector<uint8_t>& audioData);
    
    // 文本转语音
    std::vector<uint8_t> textToSpeech(const std::string& text);
};
```

## 用户界面模块API

### MainWindow类

**头文件**: `include/ui/MainWindow.h`

```cpp
class MainWindow {
public:
    // 构造函数和析构函数
    MainWindow();
    ~MainWindow();
    
    // 显示窗口
    void show();
    
    // 隐藏窗口
    void hide();
    
    // 设置窗口标题
    void setTitle(const std::string& title);
    
    // 设置窗口大小
    void setSize(int width, int height);
    
    // 注册事件处理器
    void registerEventHandler(const std::string& eventName, std::function<void()> handler);
};
```

### TodoListView类

**头文件**: `include/ui/TodoListView.h`

```cpp
class TodoListView {
public:
    // 构造函数和析构函数
    TodoListView(MainWindow* parent);
    ~TodoListView();
    
    // 显示待办事项列表
    void displayTodos(const std::vector<TodoItem>& todos);
    
    // 添加待办事项到视图
    void addTodoToView(const TodoItem& item);
    
    // 更新待办事项在视图中的显示
    void updateTodoInView(const TodoItem& item);
    
    // 从视图中删除待办事项
    void removeTodoFromView(int id);
    
    // 清空视图
    void clear();
    
    // 注册选择事件处理器
    void registerSelectionHandler(std::function<void(int)> handler);
};
```

## 工具类API

### HttpClient类

**头文件**: `include/utils/HttpClient.h`

```cpp
struct HttpResponse {
    int statusCode;
    std::string body;
    std::map<std::string, std::string> headers;
};

class HttpClient {
public:
    // 构造函数和析构函数
    HttpClient();
    ~HttpClient();
    
    // 设置超时时间
    void setTimeout(int seconds);
    
    // 添加默认请求头
    void addDefaultHeader(const std::string& name, const std::string& value);
    
    // GET请求
    HttpResponse get(const std::string& url, const std::map<std::string, std::string>& headers = {});
    
    // POST请求
    HttpResponse post(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers = {});
    
    // PUT请求
    HttpResponse put(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers = {});
    
    // DELETE请求
    HttpResponse deleteRequest(const std::string& url, const std::map<std::string, std::string>& headers = {});
};
```

### StringUtils类

**头文件**: `include/utils/StringUtils.h`

```cpp
class StringUtils {
public:
    // 字符串分割
    static std::vector<std::string> split(const std::string& str, char delimiter);
    
    // 字符串连接
    static std::string join(const std::vector<std::string>& parts, const std::string& separator);
    
    // 转换为小写
    static std::string toLowerCase(const std::string& str);
    
    // 转换为大写
    static std::string toUpperCase(const std::string& str);
    
    // 去除前后空白
    static std::string trim(const std::string& str);
    
    // 检查是否以指定字符串开头
    static bool startsWith(const std::string& str, const std::string& prefix);
    
    // 检查是否以指定字符串结尾
    static bool endsWith(const std::string& str, const std::string& suffix);
    
    // 替换字符串中的指定内容
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);
};
```

## 错误处理

项目使用异常处理机制来处理错误情况。主要的异常类包括：

### GratefulException类

**头文件**: `include/core/GratefulException.h`

```cpp
class GratefulException : public std::exception {
private:
    std::string message;
    int errorCode;
    
public:
    // 构造函数
    GratefulException(const std::string& message, int errorCode = 0);
    
    // 获取错误信息
    const char* what() const noexcept override;
    
    // 获取错误代码
    int getErrorCode() const;
};
```

### 特定异常类

- `ConfigException`: 配置相关错误
- `DbException`: 数据库相关错误
- `NetworkException`: 网络相关错误
- `AIException`: AI服务相关错误
- `UIException`: 用户界面相关错误

所有这些异常类都继承自`GratefulException`，提供统一的错误处理接口。

## 🔄 API版本控制

项目的API遵循语义化版本控制（SemVer）：
- 主版本号（Major）：不兼容的API更改
- 次版本号（Minor）：向下兼容的功能性新增
- 修订号（Patch）：向下兼容的问题修正

当前API版本：v1.0.0

## 📝 使用示例

### 基本用法示例

```cpp
// 初始化日志系统
Logger::getInstance().setLogLevel(LogLevel::INFO);
Logger::getInstance().info("Application started");

// 加载配置
if (!Configuration::getInstance().load("config.json")) {
    Logger::getInstance().error("Failed to load configuration");
    return -1;
}

// 连接数据库
if (!DbManager::getInstance().connect("database.db")) {
    Logger::getInstance().error("Failed to connect to database");
    return -1;
}

// 添加待办事项
TodoItem item("完成项目文档", "编写详细的API文档和用户手册");
item.setPriority(TodoPriority::HIGH);
TodoManager::getInstance().addTodo(item);

// 获取天气信息
WeatherService::getInstance().setApiKey("your_weather_api_key");
WeatherService::getInstance().setLocation("Beijing");
auto weather = WeatherService::getInstance().getCurrentWeather();
if (weather) {
    Logger::getInstance().info("Current temperature: " + std::to_string(weather->temperature) + "°C");
}

// 使用AI助手
AIAssistant::getInstance().setApiKey("your_ai_api_key");
std::string response = AIAssistant::getInstance().chat("请解释C++17的主要新特性");
Logger::getInstance().info("AI response: " + response);
```

## 📚 进一步阅读
- [开发者指南](development.md) - 项目开发环境和流程
- [界面设计文档](界面设计.md) - 用户界面设计说明