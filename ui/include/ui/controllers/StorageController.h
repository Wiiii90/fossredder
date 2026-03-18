/**
 * @file ui/include/ui/controllers/StorageController.h
 * @brief Declares the UI storage controller that forwards file operations to the application facade.
 */

#pragma once

#include <functional>
#include <QObject>

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes file create/open/save operations to the UI layer.
 */
class StorageController : public QObject {
    Q_OBJECT

public:
    /** @brief Creates a controller that delegates storage operations to the core application facade. */
    explicit StorageController(core::application::AppStateFacade* core, QObject* parent = nullptr);

public slots:
    void newFile(const QString& path);
    void openFile(const QString& path);
    void saveFile();
    void saveFileAs(const QString& path);

signals:
    void operationFailed(const QString& operation, const QString& error);
    void operationSucceeded(const QString& operation);

private:
    core::application::AppStateFacade* core_ = nullptr;
    bool runCoreOperation(const char* context, const std::function<void()>& action);
    void finishOperation(bool success, const QString& failureText, const QString& operation);
};

} // namespace ui
