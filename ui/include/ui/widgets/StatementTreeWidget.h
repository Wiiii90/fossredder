#pragma once

#include <QWidget>
#include <QString>
#include <QList>

#include "ui/controllers/QTMainController.h"

class QTreeView;
class QStandardItemModel;

namespace ui {

class StatementTreeWidget : public QWidget {
    Q_OBJECT
public:
    explicit StatementTreeWidget(QWidget* parent = nullptr);

signals:
    void selectionChanged(const QList<QString>& ids);
    void selectionContextChanged(ContextLevel level, const QList<QString>& ids);

private slots:
    void onSelectionChanged();

private:
    QTreeView* m_tree = nullptr;
    QStandardItemModel* m_model = nullptr;
    void setupModel();
    void addSampleData();
    ContextLevel compute_lca(const QList<QString>& ids) const;
};

}
