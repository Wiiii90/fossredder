/**
 * @file ui/include/ui/controllers/StorageController.h
 * @brief Declares the UI storage controller that forwards file operations to the application facade.
 */

#pragma once

#include <functional>
#include <QObject>
#include <QString>
#include <qqmlintegration.h>

namespace core::application { class WorkspaceFacade; }
namespace core::ports::presenters { class IWorkspacePresenter; }

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
     *  @param workspacePresenter Presenter used to normalize workspace presentation data
     *  @param parent QObject parent
     */
    explicit StorageController(core::application::WorkspaceFacade* core,
                               std::shared_ptr<core::ports::presenters::IWorkspacePresenter> workspacePresenter = {},
                               QObject* parent = nullptr);

    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)

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

    QString currentPath() const;

signals:
    void operationFailed(const QString& operation, const QString& error);
    void operationSucceeded(const QString& operation);
    void currentPathChanged();

private:
    core::application::WorkspaceFacade* core_ = nullptr;
    std::shared_ptr<core::ports::presenters::IWorkspacePresenter> workspacePresenter_;
    bool runCoreOperation(const char* context, const std::function<void()>& action);
    void finishOperation(bool success, const QString& failureText, const QString& operation);
};

} // namespace ui
