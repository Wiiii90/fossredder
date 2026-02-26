#include "ui/qml/ImportProofImageProvider.h"

#ifdef USE_QML

#include "ui/controllers/UiImportController.h"

#include <QImage>
#include <QUrl>

ImportProofImageProvider::ImportProofImageProvider(UiImportController* ctrl)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , ctrl_(ctrl)
{
}

QImage ImportProofImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    // `id` may be percent-encoded and can contain slashes.
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

#endif
