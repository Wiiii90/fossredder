#pragma once

#include <QWidget>
#include <QList>
#include <QString>

namespace ui {

class TransactionView : public QWidget {
    Q_OBJECT
public:
    explicit TransactionView(QWidget* parent = nullptr);
public slots:
    void setContext(const QList<QString>& ids);
};

}