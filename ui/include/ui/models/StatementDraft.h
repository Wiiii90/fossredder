/**
 * @file ui/include/ui/models/StatementDraft.h
 * @brief Declarations for the UI StatementDraft component.
 */

#pragma once

#include <QObject>
#include <QString>
#include <qqmlintegration.h>

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "ui/models/TransactionDraftList.h"

namespace ui {

class StatementDraft : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(StatementDraft)
    QML_UNCREATABLE("StatementDraft is exposed by the application context")
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
    Q_PROPERTY(QString draftId READ draftId WRITE setDraftId NOTIFY changed)

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY changed)
    Q_PROPERTY(int count READ count NOTIFY changed)

    Q_PROPERTY(QVariantMap current READ current NOTIFY changed)
    Q_PROPERTY(TransactionDraftList* transactions READ transactions CONSTANT)

public:
    explicit StatementDraft(QObject* parent = nullptr);

    QString name() const { return name_; }
    void setName(const QString& n);

    QString draftId() const { return draftId_; }
    void setDraftId(const QString& id);

    int currentIndex() const { return currentIndex_; }
    void setCurrentIndex(int idx);

    int count() const;

    TransactionDraftList* transactions() noexcept { return &transactions_; }

    QVariantMap current() const;
    const core::domain::catalog::WorkspaceCatalog& catalogState() const noexcept { return catalogState_; }
    bool hasCatalogState() const noexcept { return hasCatalogState_; }

    Q_INVOKABLE bool hasCurrent() const;
    Q_INVOKABLE void next();
    Q_INVOKABLE void prev();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void removeTransaction(int index);

    void setCatalogState(const core::domain::catalog::WorkspaceCatalog& state);
    void setDrafts(std::vector<TransactionDraft> drafts);

signals:
    void changed();

private:
    QString name_;
    QString draftId_;

    int currentIndex_ = 0;
    TransactionDraftList transactions_;
    core::domain::catalog::WorkspaceCatalog catalogState_;
    bool hasCatalogState_ = false;
};

}
