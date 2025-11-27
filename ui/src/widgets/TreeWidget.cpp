#include "ui/widgets/TreeWidget.h"
#include "ui/models/FakeDataGenerator.h"
#include "ui/widgets/TreeDelegate.h"
#include <QTreeView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QVBoxLayout>
#include <QStandardItem>
#include <QAbstractItemView>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QFont>
#include <algorithm>
#include <QTimer>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QDate>
#include <QVector>
#include <QLocale>

namespace ui {

TreeWidget::TreeWidget(QWidget* parent)
    : QWidget(parent)
{
    m_tree = new QTreeView(this);
    m_model = new QStandardItemModel(this);
    m_sourceModel = new QStandardItemModel(this);

    m_tree->setModel(m_model);
    // enforce single selection
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->header()->hide();

    // disable inline editing by user interaction
    m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // performance tweaks
    m_tree->setUniformRowHeights(true);
    m_tree->setAnimated(false);
    m_tree->setExpandsOnDoubleClick(false);
    m_tree->setAllColumnsShowFocus(false);
    m_tree->setAlternatingRowColors(false);
    m_tree->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);

    setupModel();
    FakeDataGenerator::populate_timeline_model(m_sourceModel, 2020, 6, 4);

    // install delegate
    m_tree->setItemDelegate(new TreeDelegate(this));

    connect(m_tree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &TreeWidget::onSelectionChanged);

    // double click zoom in
    connect(m_tree, &QTreeView::doubleClicked, this, &TreeWidget::zoomInAt);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_tree);
    setLayout(layout);

    // autoscroll timer
    m_autoScrollTimer = new QTimer(this);
    m_autoScrollTimer->setInterval(30);
    connect(m_autoScrollTimer, &QTimer::timeout, this, &TreeWidget::onAutoScrollTimer);

    // event filter for hover auto-scroll and ctrl+wheel
    m_tree->viewport()->installEventFilter(this);

    // initial years view
    buildYearsView();
}

void TreeWidget::setupModel() {
    m_model->clear();
    m_model->setHorizontalHeaderLabels({"Timeline"});
}

void TreeWidget::onSelectionChanged() {
    QList<QString> ids;
    auto selection = m_tree->selectionModel()->selectedIndexes();
    if (!selection.isEmpty()) {
        // only take the first selected index
        QModelIndex idx = selection.first();
        QVariant v = idx.data(Qt::UserRole + 1);
        if (v.isValid()) ids.append(v.toString());
    }
    emit selectionChanged(ids);

    ContextLevel level = compute_lca(selection);
    emit selectionContextChanged(level, ids);
}

ContextLevel TreeWidget::compute_lca(const QList<QModelIndex>& indexes) const {
    if (indexes.isEmpty()) return ContextLevel::Root;

    QModelIndex idx = indexes.first();
    QVariant v = idx.data(Qt::UserRole + 1);
    if (!v.isValid()) return ContextLevel::Root;
    QString id = v.toString();
    if (id.startsWith("year-")) return ContextLevel::Annual;
    if (id.startsWith("stmt-")) return ContextLevel::Statement;
    if (id.startsWith("bg-")) return ContextLevel::BookingGroup;
    if (id.startsWith("tx-")) return ContextLevel::Transaction;
    return ContextLevel::Root;
}

// build a simplified years-only view from the source model: include only years that have children
void TreeWidget::buildYearsView() {
    setupModel();
    for (int r = 0; r < m_sourceModel->rowCount(); ++r) {
        QStandardItem* src = m_sourceModel->item(r,0);
        if (!src) continue;
        QStandardItem* yearItem = new QStandardItem(src->text());
        // copy relevant data roles
        yearItem->setData(src->data(Qt::UserRole + 1), Qt::UserRole + 1);
        yearItem->setData(src->data(Qt::UserRole + 2), Qt::UserRole + 2);
        m_model->appendRow(yearItem);
    }
    m_viewLevel = ViewLevel::Years;
}

// build months view by expanding the selected year and listing months that have days
void TreeWidget::buildMonthsView(const QString& yearId) {
    setupModel();
    // extract year number for labeling
    QString yearNum = yearId;
    if (yearNum.startsWith("year-")) yearNum = yearNum.mid(5);

    // find year item in source
    for (int r = 0; r < m_sourceModel->rowCount(); ++r) {
        QStandardItem* y = m_sourceModel->item(r,0);
        if (y->data(Qt::UserRole + 1).toString() == yearId) {
            // gather months present in children (day nodes have UserRole+6 iso date)
            std::map<int, QStandardItem*> months;
            for (int i = 0; i < y->rowCount(); ++i) {
                QStandardItem* day = y->child(i,0);
                QString dateIso = day->data(Qt::UserRole + 6).toString();
                QDate d = QDate::fromString(dateIso, Qt::ISODate);
                int m = d.month();
                if (!months.count(m)) {
                    QString monthName = QLocale::system().monthName(m);
                    QString label = QString("%1 %2").arg(monthName).arg(yearNum);
                    QStandardItem* monthItem = new QStandardItem(label);
                    monthItem->setData(QStringLiteral("month-") + QString::number(m), Qt::UserRole + 1);
                    monthItem->setData(QStringLiteral("month"), Qt::UserRole + 2);
                    months[m] = monthItem;
                }
            }
            // append months to model
            for (auto& kv : months) m_model->appendRow(kv.second);
            break;
        }
    }
    m_viewLevel = ViewLevel::Months;
}

void TreeWidget::buildDaysView(const QString& yearId, int month) {
    setupModel();
    // find year item in source
    for (int r = 0; r < m_sourceModel->rowCount(); ++r) {
        QStandardItem* y = m_sourceModel->item(r,0);
        if (y->data(Qt::UserRole + 1).toString() == yearId) {
            for (int i = 0; i < y->rowCount(); ++i) {
                QStandardItem* day = y->child(i,0);
                QString dateIso = day->data(Qt::UserRole + 6).toString();
                QDate d = QDate::fromString(dateIso, Qt::ISODate);
                if (d.month() == month) {
                    // create a new day item that shows the full formatted date (e.g. "4th January 2022")
                    QString dayLabel = QString::number(d.day()) + " " + QLocale::system().monthName(d.month()) + " " + QString::number(d.year());
                    QStandardItem* clone = new QStandardItem(dayLabel);
                    // copy user roles and children
                    clone->setData(day->data(Qt::UserRole + 1), Qt::UserRole + 1);
                    clone->setData(day->data(Qt::UserRole + 2), Qt::UserRole + 2);
                    clone->setData(day->data(Qt::UserRole + 6), Qt::UserRole + 6);
                    for (int ci = 0; ci < day->rowCount(); ++ci) {
                        QStandardItem* child = day->child(ci,0)->clone();
                        clone->appendRow(child);
                    }
                    m_model->appendRow(clone);
                }
            }
            break;
        }
    }
    m_viewLevel = ViewLevel::Days;
}

void TreeWidget::zoomInAt(const QModelIndex& idx) {
    if (!idx.isValid()) return;
    QVariant idv = idx.data(Qt::UserRole + 1);
    if (!idv.isValid()) return;
    QString id = idv.toString();

    if (m_viewLevel == ViewLevel::Years) {
        // zoom to months for this year
        if (!id.startsWith("year-")) return;
        QString yearId = id;
        m_currentYear = yearId.mid(QString("year-").size());
        buildMonthsView(yearId);
    } else if (m_viewLevel == ViewLevel::Months) {
        if (!id.startsWith("month-")) return;
        int month = id.mid(QString("month-").size()).toInt();
        buildDaysView(QStringLiteral("year-") + m_currentYear, month);
    } else if (m_viewLevel == ViewLevel::Days) {
        // if double clicked on a day, expand/collapse its children (transactions)
        if (idx.isValid()) {
            m_tree->setExpanded(idx, !m_tree->isExpanded(idx));
        }
    }
}

void TreeWidget::zoomOut() {
    if (m_viewLevel == ViewLevel::Days) {
        buildMonthsView(QStringLiteral("year-") + m_currentYear);
    } else if (m_viewLevel == ViewLevel::Months) {
        buildYearsView();
    }
}

bool TreeWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_tree->viewport()) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent* we = static_cast<QWheelEvent*>(event);
            if (we->modifiers() & Qt::ControlModifier) {
                // ctrl+wheel -> zoom in/out
                int delta = we->angleDelta().y();
                if (delta > 0) {
                    // zoom in at current index under cursor
                    QModelIndex idx = m_tree->indexAt(we->position().toPoint());
                    if (!idx.isValid()) idx = m_tree->currentIndex();
                    zoomInAt(idx);
                } else {
                    zoomOut();
                }
                return true; // consume
            } else {
                // normal wheel: perform auto-scroll if near edges
                QPoint pos = m_tree->viewport()->mapFromGlobal(QCursor::pos());
                int edgeMargin = 24;
                if (pos.y() < edgeMargin) m_autoScrollDirection = -1;
                else if (pos.y() > m_tree->viewport()->height() - edgeMargin) m_autoScrollDirection = 1;
                else m_autoScrollDirection = 0;

                if (m_autoScrollDirection != 0 && !m_autoScrollTimer->isActive()) m_autoScrollTimer->start();
                if (m_autoScrollDirection == 0 && m_autoScrollTimer->isActive()) m_autoScrollTimer->stop();
                // allow default scrolling behavior too
                return false;
            }
        } else if (event->type() == QEvent::MouseMove) {
            QPoint pos = static_cast<QMouseEvent*>(event)->pos();
            int edgeMargin = 24;
            if (pos.y() < edgeMargin) m_autoScrollDirection = -1;
            else if (pos.y() > m_tree->viewport()->height() - edgeMargin) m_autoScrollDirection = 1;
            else m_autoScrollDirection = 0;

            if (m_autoScrollDirection != 0 && !m_autoScrollTimer->isActive()) m_autoScrollTimer->start();
            if (m_autoScrollDirection == 0 && m_autoScrollTimer->isActive()) m_autoScrollTimer->stop();
            return false;
        } else if (event->type() == QEvent::Leave) {
            if (m_autoScrollTimer->isActive()) m_autoScrollTimer->stop();
            m_autoScrollDirection = 0;
            return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void TreeWidget::onAutoScrollTimer() {
    if (!m_tree) return;
    if (m_autoScrollDirection == 0) return;
    QScrollBar* sb = m_tree->verticalScrollBar();
    int step = 8 * m_autoScrollDirection;
    sb->setValue(sb->value() + step);
}

}