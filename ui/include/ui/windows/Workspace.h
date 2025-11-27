#pragma once

#include <QWidget>
#include <QList>
#include <QStackedWidget>

namespace ui {

class BackgroundWidget;
class TreeWidget;
class BaseView;

class Workspace : public QWidget {
    Q_OBJECT
public:
    explicit Workspace(QWidget* parent = nullptr);

    TreeWidget* treeWidget() const;
    QWidget* centralWidget() const; // returns this
    QStackedWidget* stackedWidget() const;

    void setViews(const QList<BaseView*>& views);
    void setCurrentIndex(int idx);

private:
    TreeWidget* m_tree = nullptr;
    QStackedWidget* m_stack = nullptr;
    QWidget* m_central = nullptr;
    QList<BaseView*> m_views;
};

}
