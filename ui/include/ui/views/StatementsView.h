#pragma once

#include <QWidget>
#include <QList>
#include <QString>

namespace ui {

class StatementsView : public QWidget {
    Q_OBJECT
public:
    explicit StatementsView(QWidget* parent = nullptr);
public slots:
    void setContext(const QList<QString>& ids);
};

}