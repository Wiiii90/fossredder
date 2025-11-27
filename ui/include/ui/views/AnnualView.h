#pragma once

#include "ui/views/BaseView.h"

namespace ui {

class AnnualView : public BaseView {
    Q_OBJECT
public:
    explicit AnnualView(QWidget* parent = nullptr);
public slots:
    void setContext(const QList<QString>& ids, ContextLevel level) override;
};

}