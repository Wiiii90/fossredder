#pragma once

#include <QObject>
#include <QString>

class QWidget;

namespace ui::workflows {

class FileWorkflow : public QObject {
    Q_OBJECT

public:
    explicit FileWorkflow(QWidget* parentWindow, QObject* parent = nullptr);

public slots:
    void requestNewFile();
    void requestOpenFile();
    void requestSaveFile();
    void requestSaveFileAs();

signals:
    void newFileRequested(const QString& path);
    void openFileRequested(const QString& path);
    void saveFileRequested();
    void saveFileAsRequested(const QString& path);

private:
    QWidget* parentWindow_ = nullptr;
};

}
