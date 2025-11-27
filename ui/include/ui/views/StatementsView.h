#pragma once

#include "ui/views/BaseView.h"

namespace ui {

class StatementsView : public BaseView {
    Q_OBJECT
public:
    explicit StatementsView(QWidget* parent = nullptr);
public slots:
    void setContext(const QList<QString>& ids, ContextLevel level) override;
};

}