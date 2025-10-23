#include <fmt/format.h>
#include <kernel/DynamicLinker.h>
#include <kernel/Logger.h>
#include <map>
#include <memory>
#include <string>

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#elif defined(_WIN32)
#include <Windows.h>
#endif

#include <filesystem>
#include <kernel/Configuration.h>
#include <kernel/Extension.h>

namespace fs = std::filesystem;

struct DynamicLinker::Data
{
    std::map<std::string, std::shared_ptr<IService>> loadedServices;
    std::map<std::string, std::shared_ptr<Extension>> loadedExtensions;
};

DynamicLinker::DynamicLinker() : m_data(std::make_unique<Data>()) {}

DynamicLinker::~DynamicLinker() {}

DynamicLinker &DynamicLinker::getInstance()
{
    static DynamicLinker instance;
    return instance;
}

// 辅助函数：获取错误信息（跨平台）
std::string getLoadError()
{
#if defined(__linux__) || defined(__APPLE__)
    return dlerror() ? dlerror() : "Unknown error";
#elif defined(_WIN32)
    DWORD err = GetLastError();
    char buf[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, buf, sizeof(buf), nullptr);
    return buf;
#else
    return "Unsupported platform";
#endif
}

std::shared_ptr<IService> DynamicLinker::loadService(const std::string &serviceName)
{
    // 检查服务是否已加载
    auto it = m_data->loadedServices.find(serviceName);
    if (it != m_data->loadedServices.end())
    {
        return it->second;
    }
    auto &config = Configuration::getInstance();
    const std::string workingDir = std::get<std::string>(
        config.get("/app/working_dir", Configuration::ConfigValueType(std::string("."))));
    // 1. 构造动态库路径（建议使用绝对路径，这里仅示例）
    fs::path servicePath = fs::path(workingDir) / (serviceName + ".svc");
    if (!fs::exists(servicePath))
    {
        Logger::logError("Service file not found: {}", servicePath.string());
        return nullptr;
    }

    // 2. 加载动态库
#if defined(__linux__) || defined(__APPLE__)
    void *handle = dlopen(servicePath.string().c_str(), RTLD_LAZY);
#elif defined(_WIN32)
    HMODULE handle = LoadLibraryA(servicePath.string().c_str());
#else
    Logger::logError("Unsupported platform");
    return nullptr;
#endif

    if (!handle)
    {
        Logger::logError("Failed to load service: {}", getLoadError());
        return nullptr;
    }

    // 3. 获取创建函数（createService）
#if defined(__linux__) || defined(__APPLE__)
    typedef IService *(*CreateServiceFunc)();
    CreateServiceFunc createService = (CreateServiceFunc)dlsym(handle, "createService");
#elif defined(_WIN32)
    typedef IService *(__cdecl * CreateServiceFunc)(); // 注意调用约定，若动态库用__stdcall需匹配
    CreateServiceFunc createService = (CreateServiceFunc)GetProcAddress(handle, "createService");
#endif

    if (!createService)
    {
        Logger::logError("Failed to find createService: {}", getLoadError());
#if defined(__linux__) || defined(__APPLE__)
        dlclose(handle);
#elif defined(_WIN32)
        FreeLibrary(handle);
#endif
        return nullptr;
    }

// 4. 获取销毁函数（避免跨模块析构问题）
#if defined(__linux__) || defined(__APPLE__)
    typedef void (*DestroyServiceFunc)(IService *);
    DestroyServiceFunc destroyService = (DestroyServiceFunc)dlsym(handle, "destroyService");
#elif defined(_WIN32)
    typedef void(__cdecl * DestroyServiceFunc)(IService *);
    DestroyServiceFunc destroyService =
        (DestroyServiceFunc)GetProcAddress(handle, "destroyService");
#endif

    // 定义删除器（直接使用 lambda，不赋值给函数指针）
    auto deleter = [handle, destroyService](IService *service)
    {
        if (service)
        {
            if (destroyService)
            {
                // 优先使用动态库提供的销毁函数
                destroyService(service);
            }
            else
            {
                // 没有则使用默认 delete（注意跨模块风险）
                delete service;
            }
        }
        // 释放动态库句柄
#if defined(__linux__) || defined(__APPLE__)
        dlclose(handle);
#elif defined(_WIN32)
        FreeLibrary(handle);
#endif
    };

    // 5. 创建IService并返回带自定义删除器的shared_ptr
    IService *service = createService();
    if (!service)
    {
        Logger::logError("createService returned null");
#if defined(__linux__) || defined(__APPLE__)
        dlclose(handle);
#elif defined(_WIN32)
        FreeLibrary(handle);
#endif
        return nullptr;
    }

    auto loadedService = std::shared_ptr<IService>(service, deleter);
    m_data->loadedServices[serviceName] = loadedService;
    return loadedService;
}

std::shared_ptr<Extension> DynamicLinker::loadExtension(const std::string &extensionName)
{
    // 检查服务是否已加载
    auto it = m_data->loadedExtensions.find(extensionName);
    if (it != m_data->loadedExtensions.end())
    {
        return it->second;
    }
    auto &config = Configuration::getInstance();
    const std::string workingDir = std::get<std::string>(
        config.get("/app/working_dir", Configuration::ConfigValueType(std::string("."))));
    // 1. 构造动态库路径（建议使用绝对路径，这里仅示例）
    fs::path extensionPath = fs::path(workingDir) / (extensionName + ".ext");
    if (!fs::exists(extensionPath))
    {
        Logger::logError("Extension file not found: {}", extensionPath.string());
        return nullptr;
    }

    // 2. 加载动态库
#if defined(__linux__) || defined(__APPLE__)
    void *handle = dlopen(extensionPath.string().c_str(), RTLD_LAZY);
#elif defined(_WIN32)
    HMODULE handle = LoadLibraryA(extensionPath.string().c_str());
#else
    Logger::logError("Unsupported platform");
    return nullptr;
#endif

    if (!handle)
    {
        Logger::logError("Failed to load service: {}", getLoadError());
        return nullptr;
    }

    // 3. 获取创建函数（createService）
#if defined(__linux__) || defined(__APPLE__)
    typedef Extension *(*CreateExtensionFunc)();
    CreateExtensionFunc createExtension = (CreateExtensionFunc)dlsym(handle, "createExtension");
#elif defined(_WIN32)
    typedef Extension *(__cdecl * CreateExtensionFunc)(); // 注意调用约定，若动态库用__stdcall需匹配
    CreateExtensionFunc createExtension =
        (CreateExtensionFunc)GetProcAddress(handle, "createExtension");
#endif

    if (!createExtension)
    {
        Logger::logError("Failed to find createExtension: {}", getLoadError());
#if defined(__linux__) || defined(__APPLE__)
        dlclose(handle);
#elif defined(_WIN32)
        FreeLibrary(handle);
#endif
        return nullptr;
    }

// 4. 获取销毁函数（避免跨模块析构问题）
#if defined(__linux__) || defined(__APPLE__)
    typedef void (*DestroyExtensionFunc)(Extension *);
    DestroyExtensionFunc destroyExtension = (DestroyExtensionFunc)dlsym(handle, "destroyExtension");
#elif defined(_WIN32)
    typedef void(__cdecl * DestroyExtensionFunc)(Extension *);
    DestroyExtensionFunc destroyExtension =
        (DestroyExtensionFunc)GetProcAddress(handle, "destroyExtension");
#endif

    // 定义删除器（直接使用 lambda，不赋值给函数指针）
    auto deleter = [handle, destroyExtension](Extension *extension)
    {
        if (extension)
        {
            if (destroyExtension)
            {
                // 优先使用动态库提供的销毁函数
                destroyExtension(extension);
            }
            else
            {
                // 没有则使用默认 delete（注意跨模块风险）
                delete extension;
            }
        }
        // 释放动态库句柄
#if defined(__linux__) || defined(__APPLE__)
        dlclose(handle);
#elif defined(_WIN32)
        FreeLibrary(handle);
#endif
    };

    // 5. 创建IService并返回带自定义删除器的shared_ptr
    Extension *extension = createExtension();
    if (!extension)
    {
        Logger::logError("createExtension returned null");
#if defined(__linux__) || defined(__APPLE__)
        dlclose(handle);
#elif defined(_WIN32)
        FreeLibrary(handle);
#endif
        return nullptr;
    }

    auto loadedExtension = std::shared_ptr<Extension>(extension, deleter);
    m_data->loadedExtensions[extensionName] = loadedExtension;
    return loadedExtension;
}
