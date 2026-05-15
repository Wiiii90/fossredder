/**
 * @file ui/include/ui/platform/filesystem/FileSystemBrowser.h
 * @brief Declares the UI helper that exposes a minimal filesystem view to QML.
 */

#pragma once

#include <QObject>
#include <QVariant>
#include <QString>
#include <qqmlintegration.h>

namespace ui {

/**
 * @brief Exposes directory listing and application-directory lookup to QML.
 */
class FileSystemBrowser : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(FileSystemBrowser)
    QML_UNCREATABLE("FileSystemBrowser is provided by the application context")
public:
    /** @brief Create the filesystem helper exposed to QML. */
    explicit FileSystemBrowser(QObject* parent = nullptr);

    /** @brief List the contents of a directory in a QML-friendly payload shape.
     *  @param path Directory path
     *  @return Directory contents as QVariantList
     */
    Q_INVOKABLE QVariantList listDir(const QString& path) const;

    /** @brief Return the current application directory path.
     *  @return Application directory path
     */
    Q_INVOKABLE QString appDir() const;
};

} // namespace ui
