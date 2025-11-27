#pragma once

#include <QWidget>

class QAction;

namespace ui {

class ActionRegistry;
class BaseView;

class ToolBarWidget : public QWidget {
    Q_OBJECT
public:
    explicit ToolBarWidget(ActionRegistry* registry, QWidget* parent = nullptr);

    QWidget* viewActionArea() const;
    void registerViews(const QList<BaseView*>& views);

public slots:
    void updateViewActions(const QStringList& labels);

private:
    ActionRegistry* m_registry = nullptr;
    QWidget* m_view_action_area = nullptr;
    QList<BaseView*> m_registered_views;
};

}