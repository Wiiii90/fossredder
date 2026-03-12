/**
 * @file ui/include/ui/controllers/StorageController.h
 * @brief Declares the UI storage controller that forwards file operations to core.
 */

#pragma once

#include <functional>
#include <QObject>

#include "core/controllers/AppStateController.h"

namespace ui {

/**
 * @brief Exposes file create/open/save operations to the UI layer.
 */
class StorageController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorChanged)

public:
    /** @brief Creates a controller that delegates storage operations to the core controller. */
    explicit StorageController(core::controllers::AppStateController* core, QObject* parent = nullptr);

    /** @brief Returns the currently active storage path. */
    QString currentPath() const;
    QString lastError() const { return lastError_; }

public slots:
    void newFile(const QString& path);
    void openFile(const QString& path);
    void saveFile();
    void saveFileAs(const QString& path);

signals:
    void currentPathChanged();
    void errorChanged();
    void operationFailed(const QString& operation, const QString& error);
    void operationSucceeded(const QString& operation);

private:
    core::controllers::AppStateController* core_ = nullptr;
    QString lastError_;
    void setLastError(const QString& error);
    bool runCoreOperation(const char* context, const std::function<void()>& action);
    void finishOperation(bool success, const QString& failureText, const QString& operation);
};

}
