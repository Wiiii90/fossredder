#pragma once

#include <QObject>
#include <QVariant>
#include <QString>

namespace ui {

class FileSystemController : public QObject {
    Q_OBJECT
public:
    explicit FileSystemController(QObject* parent = nullptr);

    Q_INVOKABLE QVariantList listDir(const QString& path) const;
    Q_INVOKABLE bool exists(const QString& path) const;
    Q_INVOKABLE QString homeDir() const;
    Q_INVOKABLE QString appDir() const;
};

}
