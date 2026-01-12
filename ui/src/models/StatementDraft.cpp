#include "ui/models/StatementDraft.h"

StatementDraft::StatementDraft(QObject* parent)
    : QObject(parent), transactions_(this)
{
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
