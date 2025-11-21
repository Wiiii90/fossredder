#pragma once

#include <QWidget>
#include <QList>
#include <QString>

namespace ui {

class BookingGroupView : public QWidget {
    Q_OBJECT
public:
    explicit BookingGroupView(QWidget* parent = nullptr);
public slots:
    void setContext(const QList<QString>& ids);
};

}