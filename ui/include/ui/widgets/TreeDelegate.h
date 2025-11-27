#pragma once

#include <QStyledItemDelegate>

namespace ui {

class TreeDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit TreeDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

}