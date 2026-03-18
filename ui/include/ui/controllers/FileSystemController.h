/**
 * @file ui/include/ui/controllers/FileSystemController.h
 * @brief Declares the UI helper that exposes a minimal filesystem view to QML.
 */

#pragma once

#include <QObject>
#include <QVariant>
#include <QString>

namespace ui {

/**
 * @brief Exposes directory listing and application-directory lookup to QML.
 */
class FileSystemController : public QObject {
    Q_OBJECT
public:
    explicit FileSystemController(QObject* parent = nullptr);

    Q_INVOKABLE QVariantList listDir(const QString& path) const;
    Q_INVOKABLE QString appDir() const;
};

} // namespace ui
