/*******************************************************************************
**     FileName: Events.h
**    ClassName: Events
**       Author: Geocat & LittleBottle
**  Create Time: 2025/10/02 22:28
**  Description:
*******************************************************************************/

#ifndef EVENTS_H
#define EVENTS_H

#include <cstdint>
#include <functional>
#include <kernel/WeatherInfo.h>
#include <kernel/entities/Todo.h>
#include <string>
#include <vector>

struct SystemEvents
{
    // 系统消息事件
    struct SystemMessageEvent
    {
        uint64_t time;
        std::string message;
    };
};

struct AudioEvents
{
    // 检查是否是唤醒词的事件，由 audio service 发出，被 ai service 接收
    // 场景为：音频服务检测到活动语音，将活动语音数据传递给AI服务，由AI服务进行
    // 唤醒词检测
    struct CheckIsWakewordEvent
    {
        uint64_t time;
        std::vector<int16_t> audioData;
    };

    // 语音内容录制结束事件，由 audio service 发出，被 ai service 和 GUI
    // 接收 场景为：语音服务识别到录音结束，发送该事件
    // AIService：进行语音内容识别
    // GUI：隐藏监听蒙版组件
    struct AudioContentRecordingDoneEvent
    {
        uint64_t time;
        std::vector<int16_t> audioData;
    };

    // 音频片段事件，由 audio service 发出，被 ai service 接收，用来进行语音检测
    // 或流式处理
    struct AudioSliceEvent
    {
        uint64_t time;
        // 可以定义不同的类型，比如: 0 - 语音检测，1 - 流式音频传输
        uint8_t type;
        std::vector<char> audio_data;
    };
};

struct WeatherEvents
{
    struct WeatherUpdatedEvent
    {
        uint64_t time;
        std::string city;         // 城市名称
        WeatherInfo weather_info; // 天气信息
    };

    struct WeatherRequestEvent
    {
        uint64_t time;
    };
};

struct AIEvents
{

    // 唤醒词有效事件，由 ai service 发出，被 gui service 和 audio service 接收
    // gui 进行界面切换；
    // audio service 切换状态，进入到内容语音识别状态
    struct ValidWakeWordEvent
    {
        uint64_t time;
    };

    // 语音内容识别结果就绪事件，由 ai service 发出，被 audio service 接收
    // 场景为：AI服务识别到语音内容，将识别结果传递给音频服务，由音频服务进行
    // 状态切换
    struct SpeechRecognitionResultReadyEvent
    {
        uint64_t time;
        std::string result; // 识别结果字符串
    };

    // 智能体执行完成事件，由 ai service 发出，被 gui service 和 audio service
    // 接收 场景为：智能体执行完成，将执行结果传递给GUI服务，由GUI服务进行
    // 界面更新
    struct AIAgentDoneEvent
    {
        uint64_t time;
        std::string result; // 智能体执行结果字符串
    };
};

struct TodoEvents
{
    struct CreateTodoEvent
    {
        uint64_t time;
        Todo item; // Todo 对象
    };

    struct TodoCreatedEvent
    {
        uint64_t time;
        Todo item; // Todo 对象
    };

    struct DeleteTodoEvent
    {
        uint64_t time;
        int32_t id;        // 待办项ID
        std::string title; // 待办项标题
    };

    struct TodoDeletedEvent
    {
        uint64_t time;
        int32_t id;        // 待办项ID
        std::string title; // 待办项标题
    };

    struct UpdateTodoEvent
    {
        uint64_t time;
        int32_t id;        // 待办项ID
        std::string title; // 待办项标题
        Todo item;         // 新的Todo对象，包含更新后的字段
    };

    struct TodoUpdatedEvent
    {
        uint64_t time;
        Todo item; // Todo 对象
    };

    struct GetTodoEvent
    {
        uint64_t time;
        std::string title; // 待办项标题
        std::function<void(const Todo &)>
            receiver; // 接收函数，用于处理获取到的Todo对象
    };

    struct GetAllTodosEvent
    {
        uint64_t time;
        std::function<void(const std::vector<Todo> &)>
            receiver; // 接收函数，用于处理获取到的所有Todo对象
    };

    struct GetTodosByEvent
    {
        uint64_t time;
        std::string sql;                          // sql语句
        std::function<bool(const Todo &)> filter; // 过滤函数
        std::function<void(const std::vector<Todo> &)>
            receiver; // 接收函数，用于处理获取到的所有Todo对象
    };

    struct OrderTodosByEvent
    {
        uint64_t time;
        std::string sql;                                         // sql语句
        std::function<bool(const Todo &, const Todo &)> compare; // 比较函数
        std::function<void(const std::vector<Todo> &)>
            receiver; // 接收函数，用于处理获取到的所有Todo对象
    };
};

#endif // EVENTS_H
