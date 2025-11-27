#pragma once

#include "ui/views/BaseView.h"

namespace ui {

class BookingGroupView : public BaseView {
    Q_OBJECT
public:
    explicit BookingGroupView(QWidget* parent = nullptr);
public slots:
    void setContext(const QList<QString>& ids, ContextLevel level) override;
};

}