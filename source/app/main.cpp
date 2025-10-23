#include <QApplication>
#include <QFile>
#include <ai/AI.h>
#include <filesystem>
#include <gui/MainWindow.h>
#include <kernel/IService.h>


#include <kernel/Configuration.h>
#include <kernel/DynamicLinker.h>
#include <kernel/Logger.h>
#include <kernel/ServiceManager.h>

#include <db/DatabaseManager.h>

void initializeDatabases()
{
    auto &dbManager = DatabaseManager::getInstance();
    dbManager.getTodoConnection();
    dbManager.getModelInfoConnection("siliconflow");
}

int main(int argc, char *argv[])
{
    Configuration::getInstance().loadFromFile();
    Configuration::getInstance().set(
        "/app/name", Configuration::ConfigValueType(std::string("GratefulAssistant")));
    std::filesystem::path appPath = std::filesystem::canonical(argv[0]);
    Configuration::getInstance().set(
        "/app/working_dir", Configuration::ConfigValueType(appPath.parent_path().string()));
    initializeDatabases();

    Configuration::getInstance().saveToFile();

    ai::AI::getInstance().initialize();

    // 加载系统服务
    auto systemServices = ServiceManager::getInstance().loadSystemServices();
    for (const auto &service : systemServices)
    {
        if (service->start() && service->isRunning())
        {
            Logger::logInfo("Service {} started", service->getName());
        }
    }
    QApplication a(argc, argv);
    auto &config = Configuration::getInstance();
    std::string styleName = std::get<std::string>(config.get("/app/style", std::string("light")));
    QString stylePath = QString(":/style/%1.qss").arg(QString::fromStdString(styleName));
    QFile styleFile(stylePath);
    if (!styleFile.exists())
    {
        qWarning() << "Style file not found:" << stylePath;
    }
    if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open style file:" << stylePath;
    }
    a.setStyleSheet(styleFile.readAll());
    styleFile.close();
    MainWindow w;
    w.show();
    return a.exec();
}
