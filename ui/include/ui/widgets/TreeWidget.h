#pragma once

#include <QWidget>
#include <QString>
#include <QList>

#include "ui/controllers/QTMainController.h"

class QTreeView;
class QStandardItemModel;
class QModelIndex;
class QTimer;

namespace ui {

class TreeWidget : public QWidget {
    Q_OBJECT
public:
    explicit TreeWidget(QWidget* parent = nullptr);

signals:
    void selectionChanged(const QList<QString>& ids);
    void selectionContextChanged(ContextLevel level, const QList<QString>& ids);

private slots:
    void onSelectionChanged();
    void onAutoScrollTimer();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QTreeView* m_tree = nullptr;
    QStandardItemModel* m_model = nullptr;        // visible model
    QStandardItemModel* m_sourceModel = nullptr;  // full timeline source

    QTimer* m_autoScrollTimer = nullptr;
    int m_autoScrollDirection = 0; // -1 up, 1 down, 0 none

    enum class ViewLevel { Years, Months, Days };
    ViewLevel m_viewLevel = ViewLevel::Years;
    QString m_currentYear; // "2025"
    int m_currentMonth = 0; // 1-12
    bool m_zoomMode = true; // if true use zoom behavior, otherwise classic tree

    void setupModel();
    void addSampleData();
    ContextLevel compute_lca(const QList<QModelIndex>& indexes) const;

    // view builders
    void buildYearsView();
    void buildMonthsView(const QString& yearId);
    void buildDaysView(const QString& yearId, int month);

    // interactions
    void zoomInAt(const QModelIndex& idx);
    void zoomOut();

    // utilities
    QString monthFromDateString(const QString& isoDate) const;
};

}