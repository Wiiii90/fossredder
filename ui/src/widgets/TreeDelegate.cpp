#include "ui/widgets/TreeDelegate.h"
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QFont>
#include <QFontMetrics>

namespace ui {

TreeDelegate::TreeDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void TreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    painter->save();

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    QString displayText = opt.text;
    opt.text.clear();

    const QWidget* widget = opt.widget;
    QStyle* style = widget ? widget->style() : QApplication::style();

    // draw background/selection only using style (no custom color/glow)
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    QRect rect = option.rect;
    const int iconSize = 16;
    const int padding = 6;

    painter->setClipRect(rect);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    // determine item type
    QString type = index.data(Qt::UserRole + 2).toString();

    // Custom header style for year/month
    if (type == "year" || type == "month") {
        // subtle background for header
        QColor bg = option.palette.window().color();
        bg.setAlpha(10);
        painter->fillRect(rect.adjusted(0, 2, 0, -2), bg);

        // draw centered title with lines
        QFont f = option.font;
        f.setBold(true);
        painter->setFont(f);
        QString text = displayText.isEmpty() ? index.data(Qt::DisplayRole).toString() : displayText;

        QFontMetrics fm(f);
        int textW = fm.horizontalAdvance(text);
        int centerX = rect.left() + rect.width()/2;
        int y = rect.top() + (rect.height()+fm.ascent()-fm.descent())/2;

        int lineLen = qMax(20, (rect.width()/2 - textW/2 - 12));
        QPoint left1(rect.left() + rect.width()/2 - lineLen - textW/2 - 8, y - 1);
        QPoint left2(rect.left() + rect.width()/2 - textW/2 - 8, y - 1);
        QPoint right1(rect.left() + rect.width()/2 + textW/2 + 8, y - 1);
        QPoint right2(rect.left() + rect.width()/2 + lineLen + textW/2 + 8, y - 1);

        QPen pen(option.palette.mid().color());
        pen.setWidth(1);
        painter->setPen(pen);
        painter->drawLine(left1, left2);
        painter->drawLine(right1, right2);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(QRect(rect.left(), rect.top(), rect.width(), rect.height()), Qt::AlignCenter, text);
        painter->restore();
        return;
    }

    QRect iconRect(rect.left() + padding, rect.top() + (rect.height() - iconSize) / 2, iconSize, iconSize);

    // cache pixmaps locally (static) to avoid repeated lookup
    static QPixmap pixYear, pixBlank, pixBG, pixTx;
    if (pixYear.isNull()) {
        QStyle* s = QApplication::style();
        pixYear = s->standardIcon(QStyle::SP_DirIcon).pixmap(iconSize, iconSize);
        pixBlank = QPixmap(iconSize, iconSize);
        pixBlank.fill(Qt::transparent);
        pixBG = s->standardIcon(QStyle::SP_DirOpenIcon).pixmap(iconSize, iconSize);
        pixTx = s->standardIcon(QStyle::SP_ArrowRight).pixmap(iconSize, iconSize);
    }

    const QPixmap* usePix = &pixBlank;
    // only show an icon for year (annual) nodes; everything else uses blank pixmap
    if (type == "year") usePix = &pixYear;

    if (!usePix->isNull()) painter->drawPixmap(iconRect, *usePix);

    int textX = iconRect.right() + padding;
    QRect textRect(textX, rect.top(), rect.width() - textX - padding, rect.height());

    bool isDraft = false;
    QVariant d = index.data(Qt::UserRole + 3);
    if (d.isValid() && d.toString() == "draft") isDraft = true;

    QFont font = option.font;
    if (isDraft) font.setItalic(true);

    painter->setFont(font);
    QColor txtColor = option.palette.color(QPalette::Text);
    painter->setPen(txtColor);

    QString text = displayText.isEmpty() ? index.data(Qt::DisplayRole).toString() : displayText;
    QFontMetrics fm(font);
    QString elided = fm.elidedText(text, Qt::ElideRight, textRect.width());
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);

    painter->restore();
}

QSize TreeDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(qMax(s.height(), 20));
    return s;
}

}