#pragma once

#include <QObject>
#include <QVariant>
#include <QString>

class UiFileSystem : public QObject {
    Q_OBJECT
public:
    explicit UiFileSystem(QObject* parent = nullptr);

    Q_INVOKABLE QVariantList listDir(const QString& path) const;
    Q_INVOKABLE bool exists(const QString& path) const;
    Q_INVOKABLE QString homeDir() const;
    Q_INVOKABLE QString appDir() const;
};
