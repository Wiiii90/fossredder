#include "ui/providers/DraftProofProvider.h"

#ifdef USE_QML

#include "ui/controllers/ImportController.h"

#include <QImage>
#include <QUrl>

namespace ui {

DraftProofProvider::DraftProofProvider(ImportController* ctrl)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , ctrl_(ctrl)
{
}

QImage DraftProofProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    const QString key = QUrl::fromPercentEncoding(id.toUtf8());

    QByteArray bytes;
    if (ctrl_) bytes = ctrl_->artifactBytes(key);

    QImage img;
    if (!bytes.isEmpty()) {
        img.loadFromData(bytes);
    }

    if (requestedSize.isValid() && !img.isNull()) {
        img = img.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    if (size) *size = img.size();
    return img;
}

}

#endif
