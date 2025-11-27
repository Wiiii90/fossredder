#pragma once

#include <QObject>
#include <QList>
#include <QString>

namespace ui {

enum class ContextLevel {
    Root,
    Annual,
    Statement,
    BookingGroup,
    Transaction
};

Q_DECLARE_METATYPE(ContextLevel)

class QTMainController : public QObject {
    Q_OBJECT
public:
    explicit QTMainController(QObject* parent = nullptr);

signals:
    void contextChanged(ContextLevel level, const QList<QString>& ids);

public slots:
    void onTreeSelectionChanged(ContextLevel level, const QList<QString>& ids);

private:
    ContextLevel compute_lca(const QList<QString>& ids) const;
};

}
