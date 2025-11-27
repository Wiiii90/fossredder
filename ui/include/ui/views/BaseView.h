#pragma once

#include <QWidget>
#include <QList>
#include <QString>

#include "ui/controllers/QTMainController.h"

class QLabel;
class QWidget;

namespace ui {

class BaseView : public QWidget {
    Q_OBJECT
public:
    explicit BaseView(QWidget* parent = nullptr);

public slots:
    virtual void setContext(const QList<QString>& ids, ContextLevel level);

signals:
    void actionsChanged(const QStringList& labels);

protected:
    QLabel* breadcrumb_ = nullptr;
    QWidget* action_area_ = nullptr;
    QWidget* content_area_ = nullptr;

    void set_actions(const QStringList& labels);
    void clear_actions();
};

}
