#pragma once

#include <QWidget>
#include <QList>
#include <QString>

namespace ui {

class YearlyStatementView : public QWidget {
    Q_OBJECT
public:
    explicit YearlyStatementView(QWidget* parent = nullptr);
public slots:
    void setContext(const QList<QString>& ids);
};

}