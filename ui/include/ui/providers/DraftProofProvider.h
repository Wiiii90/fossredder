#pragma once

#ifdef USE_QML

#include <QPointer>
#include <QQuickImageProvider>

namespace ui { class ImportController; }

namespace ui {

class DraftProofProvider : public QQuickImageProvider {
public:
    explicit DraftProofProvider(ImportController* ctrl);

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    QPointer<ImportController> ctrl_;
};

}

#endif
