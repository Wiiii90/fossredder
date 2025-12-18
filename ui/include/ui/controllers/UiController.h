#pragma once

#include <QObject>
#include <memory>

class StatementController;

class UiController : public QObject
{
    Q_OBJECT
public:
    explicit UiController(QObject* parent = nullptr);

signals:
    void importRequested(const QString& path);

public slots:
    void import();
    void exportPath();
    void about();

private:
    std::shared_ptr<StatementController> coreController_;
};
