#pragma once

#ifdef USE_QML

#include <QPointer>
#include <QQuickImageProvider>

namespace ui { class ImportController; }

class DraftProofProvider : public QQuickImageProvider {
public:
    explicit DraftProofProvider(ui::ImportController* ctrl);

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    QPointer<ui::ImportController> ctrl_;
};

#endif
