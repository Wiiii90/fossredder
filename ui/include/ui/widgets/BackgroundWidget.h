#pragma once

#include <QWidget>

namespace ui {

class BackgroundWidget : public QWidget {
    Q_OBJECT
public:
    explicit BackgroundWidget(QWidget* parent = nullptr);
protected:
    void paintEvent(QPaintEvent* ev) override;
};

}
