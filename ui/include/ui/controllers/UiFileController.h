#pragma once

#include <QObject>

#include "core/controllers/FileController.h"
#include "core/models/DeletionImpact.h"

class UiFileController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)

public:
    explicit UiFileController(FileController* core, QObject* parent = nullptr);

    QString currentPath() const;

public slots:
    void newFile(const QString& path);
    void openFile(const QString& path);
    void saveFile();
    void saveFileAs(const QString& path);

signals:
    void currentPathChanged();
    void stateLoaded();
    void deletionImpact(const DeletionImpact& impact);

private:
    FileController* core_;
};
