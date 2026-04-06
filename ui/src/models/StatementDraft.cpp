/**
 * @file ui/src/models/StatementDraft.cpp
 * @brief Implementation of the UI StatementDraft component.
 */

#include "ui/models/StatementDraft.h"

#include <QAbstractItemModel>

namespace ui {

StatementDraft::StatementDraft(QObject* parent)
    : QObject(parent), transactions_()
{
    QObject::connect(&transactions_, &QAbstractItemModel::dataChanged, this, [this]() { emit changed(); });
    QObject::connect(&transactions_, &QAbstractItemModel::rowsInserted, this, [this]() { emit changed(); });
    QObject::connect(&transactions_, &QAbstractItemModel::rowsRemoved, this, [this]() { emit changed(); });
    QObject::connect(&transactions_, &QAbstractItemModel::modelReset, this, [this]() { emit changed(); });
}

void StatementDraft::setName(const QString& n)
{
    if (name_ == n) return;
    name_ = n;
    emit changed();
}

void StatementDraft::setCurrentIndex(int idx)
{
    if (idx < 0) idx = 0;
    const int c = count();
    if (c > 0 && idx >= c) idx = c - 1;
    if (currentIndex_ == idx) return;
    currentIndex_ = idx;
    emit changed();
}

int StatementDraft::count() const
{
    return transactions_.rowCount();
}

QVariantMap StatementDraft::current() const
{
    return transactions_.get(currentIndex_);
}

void StatementDraft::setCatalogState(const core::domain::AppState& state)
{
    catalogState_ = state;
    hasCatalogState_ = true;
}

bool StatementDraft::hasCurrent() const
{
    return count() > 0 && currentIndex_ >= 0 && currentIndex_ < count();
}

void StatementDraft::next()
{
    if (!hasCurrent()) return;
    setCurrentIndex(currentIndex_ + 1);
}

void StatementDraft::prev()
{
    if (!hasCurrent()) return;
    setCurrentIndex(currentIndex_ - 1);
}

void StatementDraft::setDrafts(std::vector<TransactionDraft> drafts)
{
    transactions_.setDrafts(std::move(drafts));
    if (currentIndex_ >= count()) currentIndex_ = std::max(0, count() - 1);
    emit changed();
}

void StatementDraft::refresh()
{
    emit changed();
}

}
