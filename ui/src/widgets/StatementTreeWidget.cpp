#include "ui/widgets/StatementTreeWidget.h"
#include <QTreeView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QVBoxLayout>
#include <QStandardItem>
#include <QAbstractItemView>

namespace ui {

StatementTreeWidget::StatementTreeWidget(QWidget* parent)
    : QWidget(parent)
{
    m_tree = new QTreeView(this);
    m_model = new QStandardItemModel(this);

    m_tree->setModel(m_model);
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tree->header()->hide();

    setupModel();
    addSampleData();

    connect(m_tree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &StatementTreeWidget::onSelectionChanged);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_tree);
    setLayout(layout);
}

void StatementTreeWidget::setupModel() {
    m_model->clear();
    m_model->setHorizontalHeaderLabels({"Statements"});
}

void StatementTreeWidget::addSampleData() {
    QStandardItem* rootItem = m_model->invisibleRootItem();

    QStandardItem* stmt1 = new QStandardItem("Statement 2024-01-01");
    stmt1->setData(QStringLiteral("stmt-2024-01-01"), Qt::UserRole + 1);
    stmt1->setData(QStringLiteral("persisted"), Qt::UserRole + 2);

    QStandardItem* bg1 = new QStandardItem("BG 2024-01-01 A");
    bg1->setData(QStringLiteral("bg-1"), Qt::UserRole + 1);
    bg1->setData(QStringLiteral("persisted"), Qt::UserRole + 2);

    QStandardItem* t1 = new QStandardItem("Transaction 1");
    t1->setData(QStringLiteral("tx-1"), Qt::UserRole + 1);
    t1->setData(QStringLiteral("persisted"), Qt::UserRole + 2);

    QStandardItem* t2 = new QStandardItem("Transaction 2 (draft)");
    t2->setData(QStringLiteral("tx-2"), Qt::UserRole + 1);
    t2->setData(QStringLiteral("draft"), Qt::UserRole + 2);
    t2->setBackground(QColor(255, 248, 220));

    QStandardItem* bg2 = new QStandardItem("BG 2024-01-01 B");
    bg2->setData(QStringLiteral("bg-2"), Qt::UserRole + 1);
    bg2->setData(QStringLiteral("persisted"), Qt::UserRole + 2);

    QStandardItem* t3 = new QStandardItem("Transaction 3");
    t3->setData(QStringLiteral("tx-3"), Qt::UserRole + 1);
    t3->setData(QStringLiteral("persisted"), Qt::UserRole + 2);

    bg1->appendRow(t1);
    bg1->appendRow(t2);
    bg2->appendRow(t3);

    stmt1->appendRow(bg1);
    stmt1->appendRow(bg2);

    rootItem->appendRow(stmt1);

    m_tree->expandAll();
}

void StatementTreeWidget::onSelectionChanged() {
    QList<QString> ids;
    auto selection = m_tree->selectionModel()->selectedIndexes();
    for (const QModelIndex& idx : selection) {
        QVariant v = idx.data(Qt::UserRole + 1);
        if (v.isValid()) ids.append(v.toString());
    }
    emit selectionChanged(ids);
    ContextLevel level = compute_lca(ids);
    emit selectionContextChanged(level, ids);
}

ContextLevel StatementTreeWidget::compute_lca(const QList<QString>& ids) const {
    if (ids.isEmpty()) return ContextLevel::Root;
    bool all_stmt = true;
    bool all_bg = true;
    bool all_tx = true;
    for (const QString& id : ids) {
        if (!id.startsWith("stmt-")) all_stmt = false;
        if (!id.startsWith("bg-")) all_bg = false;
        if (!id.startsWith("tx-")) all_tx = false;
    }
    if (all_tx) return ContextLevel::Transaction;
    if (all_bg) return ContextLevel::BookingGroup;
    if (all_stmt) return ContextLevel::Statement;
    return ContextLevel::BookingGroup;
}

}
