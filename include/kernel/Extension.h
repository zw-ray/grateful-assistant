/*******************************************************************************
**     FileName: Extension.h
**    ClassName: Extension
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/06 09:19
**  Description:
*******************************************************************************/

#ifndef EXTENSION_H
#define EXTENSION_H

#include <cstdint>
#include <kernel/KernelExport.h>
#include <string>
#include <vector>

class KERNEL_API Extension
{
public:
    Extension();
    virtual ~Extension();

    virtual void initialize() = 0;
    virtual void finalize() = 0;
    virtual std::string name() const = 0;

    struct KERNEL_API ExtensionInfo
    {
        uint32_t apiVersion;
        std::string name;
        std::string type;
        std::string path;

        struct Dependency
        {
            std::string name;
            std::string version;
        };
        std::vector<Dependency> dependencies;
        struct Detail
        {
            std::string author;
            std::string version;
            std::string description;
            std::vector<Dependency> dependencies;
            std::string license;
            enum PlatformTriplet : uint32_t
            {
                // platforms - right 16 bits
                kWindows = 1,
                kLinux = 2,
                kMacOS = 3,
                kAndroid = 4,
                kIOS = 5,

                // architectures - left 16 bits
                kX86 = 1,
                kX64 = 2,
                kARM = 3,
                kARM64 = 4,

                // triplets
                kWindowsX86 = kWindows | (kX86 << 16),
                kWindowsX64 = kWindows | (kX64 << 16),
                kWindowsARM = kWindows | (kARM << 16),
                kWindowsARM64 = kWindows | (kARM64 << 16),
                kLinuxX86 = kLinux | (kX86 << 16),
                kLinuxX64 = kLinux | (kX64 << 16),
                kLinuxARM = kLinux | (kARM << 16),
                kLinuxARM64 = kLinux | (kARM64 << 16),
                kMacOSX86 = kMacOS | (kX86 << 16),
                kMacOSX64 = kMacOS | (kX64 << 16),
                kMacOSARM = kMacOS | (kARM << 16),
                kMacOSARM64 = kMacOS | (kARM64 << 16),
            };
            uint32_t platforms;
            std::string url;
            std::string contact;
            std::vector<std::string> keywords;

            enum Category
            {
                kSystem,
                kAI,
                kAudio,
                kFile,
                kGame,
                kMusic,
                kNovel,
                kSysc,
                kTodo,
                kVideo,
                kWeather,
                kWeb,
                kWriter,
                // other categories
                kOther,
            };
            Category category;
        } detail;
    };
    virtual ExtensionInfo info() const = 0;

protected:
    ExtensionInfo m_info;
}; // class Extension

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#ifndef EXTENSION_EXPORT
#define EXTENSION_EXPORT __declspec(dllexport)
#endif
#else
#ifndef EXTENSION_EXPORT
#define EXTENSION_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifndef EXTENSION_EXPORT_FUNCTION
#define EXTENSION_EXPORT_FUNCTION()                                                                \
    extern "C" EXTENSION_EXPORT Extension *createExtension();                                      \
    extern "C" EXTENSION_EXPORT void destroyExtension(Extension *extension);
#endif // EXTENSION_EXPORT_FUNCTION

#endif // EXTENSION_H
