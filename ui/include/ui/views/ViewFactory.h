#pragma once

#include <QList>

class QWidget;

namespace ui {

class BaseView;

class ViewFactory {
public:
    static QList<BaseView*> createViews(QWidget* parent);
    static int maxSlotsForViews(const QList<BaseView*>& views);
};

}