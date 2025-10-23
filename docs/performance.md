# ⚡ 性能优化建议（v0.1.0）

本指南聚焦构建速度、启动时间、GUI 响应与 I/O 性能优化。

## 构建与链接
- 使用 `-DCMAKE_BUILD_TYPE=Release` 与 `-j` 并行编译。
- 静态库/共享库选择：根据部署场景切换 `BUILD_SHARED_LIBS`。
- 启用 Ninja（可选）：`-G Ninja` 加快增量构建。

## 资源与 GUI
- 启用 `AUTOMOC/AUTORCC/AUTOUIC` 减少手工步骤与错误。
- 合理使用 `QTimer` 与消息队列避免 UI 阻塞（已用于系统消息）。
- 控制浮动消息最大数量与间距，避免过度绘制（`MAX_MESSAGES=3`）。

## 日志与事件
- 调试模式减少日志输出量；生产使用 Info/Warn/Error 级别。
- 事件总线回调避免重型计算，必要时使用后台线程处理。

## 数据库与 I/O
- 读取频繁数据使用缓存；批量写入减少事务次数。
- SQLite：合适的 Journal 模式与同步设置（视需求）。

## 网络与 AI Provider
- 参数合理：`max_tokens`、`temperature` 等根据任务调优。
- 失败重试与超时控制，避免 GUI 阻塞。

## 打包与分发（Windows）
- 使用 `windeployqt` 自动拷贝依赖，减少运行时缺库问题。

## 基准与监控
- 启动时间基准：记录 `MainWindow` 初始化耗时。
- GUI 响应度：模拟高频消息队列并观察绘制性能。