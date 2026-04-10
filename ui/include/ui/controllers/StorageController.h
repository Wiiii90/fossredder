/**
 * @file ui/include/ui/controllers/StorageController.h
 * @brief Declares the UI storage controller that forwards file operations to the application facade.
 */

#pragma once

#include <functional>
#include <QObject>
#include <qqmlintegration.h>

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes file create/open/save operations to the UI layer.
 */
class StorageController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(StorageController)
    QML_UNCREATABLE("StorageController is provided by the application context")

public:
    /** @brief Creates a controller that delegates storage operations to the core application facade.
     *  @param core Core application facade pointer
     *  @param parent QObject parent
     */
    explicit StorageController(core::application::AppStateFacade* core, QObject* parent = nullptr);

public slots:
    /** @brief Create a new file at the requested path.
     *  @param path File path to create
     */
    void newFile(const QString& path);

    /** @brief Open an existing file from the requested path.
     *  @param path File path to open
     */
    void openFile(const QString& path);

    /** @brief Save the currently open file. */
    void saveFile();

    /** @brief Save the current file under a new path.
     *  @param path New file path
     */
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
