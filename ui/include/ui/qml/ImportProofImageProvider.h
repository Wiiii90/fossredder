#pragma once

#ifdef USE_QML

#include <QPointer>
#include <QQuickImageProvider>

class UiImportController;

class ImportProofImageProvider : public QQuickImageProvider {
public:
    explicit ImportProofImageProvider(UiImportController* ctrl);

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    QPointer<UiImportController> ctrl_;
};

#endif
