# 贡献指南

感谢您考虑为Grateful Assistant项目做出贡献！这份指南将帮助您了解如何参与项目开发、提交代码和报告问题。

## 📋 行为准则

在参与本项目之前，请阅读我们的[行为准则](CODE_OF_CONDUCT.md)。我们希望所有参与者都能尊重他人，共同营造一个友好、包容的社区环境。

## 🚀 开始贡献

### 环境准备

请参考[开发者指南](docs/development.md)中的环境搭建部分，设置开发环境。

### 寻找贡献机会

您可以通过以下方式找到贡献机会：
- 浏览GitHub仓库中的[Issues](https://github.com/yourusername/gratefulassistant/issues)列表
- 查找标记为"good first issue"或"help wanted"的问题
- 提出新功能建议或报告bug
- 改进文档或添加示例

### 创建分支

在开始工作之前，请创建一个新的分支：

```bash
# 确保您的main分支是最新的
git checkout main
git pull origin main

# 创建一个新的分支
# 对于功能：git checkout -b feature/your-feature-name
# 对于修复：git checkout -b fix/your-bugfix-name
git checkout -b feature/your-feature-name
```

## 🔧 开发流程

### 编码规范

请遵循项目的[编码规范](docs/development.md#代码规范)：
- 采用4个空格进行缩进
- 花括号放在行尾
- 类名使用PascalCase命名法
- 函数和变量使用camelCase命名法
- 私有成员变量以下划线（_）开头
- 每行代码不超过100个字符
- 使用现代C++特性（C++17/20）
- 添加适当的注释，特别是公共接口

### 编写测试

为您的代码编写测试，确保功能正常且不会破坏现有功能：
- 为新功能添加单元测试
- 确保所有测试在提交前通过
- 尽量提高测试覆盖率

### 提交代码

在提交代码时，请遵循以下规范：

1. **保持提交粒度适中**：每个提交应包含一个逻辑上的更改
2. **编写清晰的提交信息**：
   - 第一行：简短描述（不超过50个字符）
   - 空行
   - 详细描述（如有必要）
3. **遵循提交消息格式**：
   ```
   <类型>: <简短描述>
   
   <详细描述（如有）>
   
   关联的Issue: #<issue编号>
   ```

提交类型包括：
- `feat`: 新功能
- `fix`: 修复bug
- `docs`: 文档更改
- `style`: 代码格式更改（不影响代码功能）
- `refactor`: 代码重构（不添加功能或修复bug）
- `perf`: 性能优化
- `test`: 添加或修改测试
- `chore`: 构建过程或辅助工具变动

### 推送到远程仓库

```bash
git push origin feature/your-feature-name
```

## 📝 创建Pull Request

当您完成工作并准备提交时，请创建一个Pull Request：

1. 在GitHub上导航到项目仓库
2. 点击"Pull requests"选项卡
3. 点击"New pull request"按钮
4. 选择您的分支作为比较分支
5. 添加详细的描述，解释您的更改、解决的问题以及如何测试
6. 关联相关的Issue（如`Fixes #123`或`Closes #456`）
7. 点击"Create pull request"按钮

## 🔍 代码审查

提交Pull Request后，项目维护者将审查您的代码。在审查过程中，您可能需要根据反馈进行一些修改。请及时更新您的代码并响应审查意见。

### 审查流程

1. 自动化测试：GitHub Actions或其他CI系统将运行测试
2. 代码审查：维护者检查代码质量、风格和功能
3. 合并：当审查通过且所有测试通过后，您的代码将被合并

## 🐛 报告问题

如果您发现bug或有新功能建议，请在GitHub上创建一个Issue：

1. 点击"Issues"选项卡
2. 点击"New issue"按钮
3. 选择合适的模板（bug报告或功能建议）
4. 填写详细信息，包括：
   - 问题的描述
   - 复现步骤（对于bug）
   - 期望的行为
   - 实际的行为
   - 环境信息（操作系统、编译器版本等）
   - 截图或日志（如有）
5. 点击"Submit new issue"按钮

## 📚 文档贡献

改进文档也是非常重要的贡献：
- 修正拼写或语法错误
- 添加缺失的文档
- 更新过时的信息
- 添加示例代码或使用场景
- 翻译文档（如支持多语言）

## 📞 联系我们

如果您有任何问题或需要帮助，可以通过以下方式联系项目维护者：
- GitHub Issues: [https://github.com/yourusername/gratefulassistant/issues](https://github.com/yourusername/gratefulassistant/issues)
- [其他联系方式，如邮件列表或聊天群组]

## 🎉 感谢

再次感谢您对Grateful Assistant项目的贡献！您的参与对于项目的成功至关重要。