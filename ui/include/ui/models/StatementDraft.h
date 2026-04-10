/**
 * @file ui/include/ui/models/StatementDraft.h
 * @brief Declarations for the UI StatementDraft component.
 */

#pragma once

#include <QObject>
#include <QString>
#include <qqmlintegration.h>

#include "core/models/AppState.h"
#include "ui/models/TransactionDraftList.h"

namespace ui {

class StatementDraft : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(StatementDraft)
    QML_UNCREATABLE("StatementDraft is exposed by the application context")
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY changed)
    Q_PROPERTY(int count READ count NOTIFY changed)

    Q_PROPERTY(QVariantMap current READ current NOTIFY changed)
    Q_PROPERTY(TransactionDraftList* transactions READ transactions CONSTANT)

public:
    explicit StatementDraft(QObject* parent = nullptr);

    QString name() const { return name_; }
    void setName(const QString& n);

    int currentIndex() const { return currentIndex_; }
    void setCurrentIndex(int idx);

    int count() const;

    TransactionDraftList* transactions() noexcept { return &transactions_; }

    QVariantMap current() const;
    const core::domain::AppState& catalogState() const noexcept { return catalogState_; }
    bool hasCatalogState() const noexcept { return hasCatalogState_; }

    Q_INVOKABLE bool hasCurrent() const;
    Q_INVOKABLE void next();
    Q_INVOKABLE void prev();
    Q_INVOKABLE void refresh();

    void setCatalogState(const core::domain::AppState& state);
    void setDrafts(std::vector<TransactionDraft> drafts);

signals:
    void changed();

private:
    QString name_;

    int currentIndex_ = 0;
    TransactionDraftList transactions_;
    core::domain::AppState catalogState_;
    bool hasCatalogState_ = false;
};

}
