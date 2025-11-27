#include "ui/widgets/BackgroundWidget.h"
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>

namespace ui {

BackgroundWidget::BackgroundWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void BackgroundWidget::paintEvent(QPaintEvent* ev) {
    Q_UNUSED(ev);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    QRectF r = rect();
    qreal maxDim = qMax(r.width(), r.height());

    // Base: very light, nearly white canvas with a soft cool tint
    p.fillRect(r, QColor(250, 251, 254));

    // Replace distinct blobs with two very large, extremely soft blue gradients
    // that overlap smoothly. Radii are large and alphas very low to avoid visible spots.
    QRadialGradient blueA(QPointF(r.width()*0.30, r.height()*0.22), maxDim * 2.2);
    blueA.setColorAt(0.0, QColor(43,108,176,18));
    blueA.setColorAt(0.6, QColor(43,108,176,9));
    blueA.setColorAt(1.0, QColor(43,108,176,0));
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.fillRect(r, QBrush(blueA));

    QRadialGradient blueB(QPointF(r.width()*0.72, r.height()*0.72), maxDim * 1.9);
    blueB.setColorAt(0.0, QColor(60,140,200,16));
    blueB.setColorAt(0.6, QColor(60,140,200,8));
    blueB.setColorAt(1.0, QColor(60,140,200,0));
    p.fillRect(r, QBrush(blueB));

    // Very soft, broad warm overlay (orange) blended subtly across the whole area
    // low alpha and large radius so it tints rather than forms a spot
    QRadialGradient warm(QPointF(r.width()*0.5, r.height()*0.38), maxDim * 2.0);
    warm.setColorAt(0.0, QColor(255,165,90,24));
    warm.setColorAt(0.5, QColor(255,165,90,10));
    warm.setColorAt(1.0, QColor(255,165,90,0));
    p.fillRect(r, QBrush(warm));

    // Final very soft neutral overlay to harmonize tones and mute contrasts
    QLinearGradient unify(r.topLeft(), r.bottomRight());
    unify.setColorAt(0.0, QColor(255,255,255,8));
    unify.setColorAt(1.0, QColor(245,246,250,8));
    p.fillRect(r, QBrush(unify));
}

}
