/**
 * @file ui/src/adapters/core/AnnualResultMapper.cpp
 * @brief Maps annual core results into UI payloads and applies presentation ordering.
 */

#include "ui/adapters/core/AnnualResultMapper.h"

#include <algorithm>

#include <QVariantList>

#include "core/ports/presenters/IAnnualPresenter.h"
#include "ui/shared/payload/PayloadMapper.h"

namespace ui::annual {

namespace {

class AnnualPresenter final : public core::ports::presenters::IAnnualPresenter {
public:
    core::application::annual::AnnualResult present(const core::application::annual::AnnualResult& result) const override
    {
        auto projected = result;
        auto byDateAmount = [](const auto& lhs, const auto& rhs) {
            if (lhs.bookingDate != rhs.bookingDate) return lhs.bookingDate < rhs.bookingDate;
            if (lhs.amount != rhs.amount) return lhs.amount < rhs.amount;
            return lhs.transactionId < rhs.transactionId;
        };
        std::sort(projected.deduplicated.begin(), projected.deduplicated.end(), byDateAmount);
        std::sort(projected.similar.begin(), projected.similar.end(), byDateAmount);
        std::sort(projected.divergent.begin(), projected.divergent.end(), byDateAmount);
        std::sort(projected.workspaceOnly.begin(), projected.workspaceOnly.end(), byDateAmount);
        return projected;
    }
};

QVariantMap toRow(const core::application::annual::AnnualRowResult& row)
{
    QVariantMap out;
    out.insert(QStringLiteral("key"), QString::fromStdString(row.key));
    out.insert(QStringLiteral("id"), QString::fromStdString(row.transactionId));
    out.insert(QStringLiteral("name"), QString::fromStdString(row.transactionName));
    out.insert(QStringLiteral("bookingDate"), QString::fromStdString(row.bookingDate));
    out.insert(QStringLiteral("amount"), row.amount);
    out.insert(QStringLiteral("status"), row.status);
    out.insert(QStringLiteral("statusText"),
               row.status == 1 ? QStringLiteral("Unverified")
                               : row.status == 2 ? QStringLiteral("Verified")
                                                 : row.status == 3 ? QStringLiteral("Completed")
                                                                   : QStringLiteral("Neutral"));
    out.insert(QStringLiteral("allocatable"), row.allocatable);
    out.insert(QStringLiteral("contractId"), QString::fromStdString(row.contractId));
    out.insert(QStringLiteral("contractType"), QString::fromStdString(row.contractType));
    out.insert(QStringLiteral("statementId"), QString::fromStdString(row.statementId));
    out.insert(QStringLiteral("isMissingLive"), row.missingLive);
    out.insert(QStringLiteral("isMixedYear"), row.mixedYear);
    out.insert(QStringLiteral("isCalcVariant"), row.calcVariant);
    out.insert(QStringLiteral("isDuplicate"), row.duplicateCount > 1);
    out.insert(QStringLiteral("duplicateCount"), row.duplicateCount);
    out.insert(QStringLiteral("sourceAnalysisIds"), payload::mapper::toVariantStringList(row.sourceAnalysisIds));
    out.insert(QStringLiteral("sourceAnalysisNames"), payload::mapper::toVariantStringList(row.sourceAnalysisNames));
    return out;
}

QVariantList toRows(const std::vector<core::application::annual::AnnualRowResult>& rows)
{
    QVariantList out;
    out.reserve(static_cast<int>(rows.size()));
    for (const auto& row : rows) out.push_back(toRow(row));
    return out;
}

const AnnualPresenter& presenter()
{
    static const AnnualPresenter instance;
    return instance;
}

} // namespace

core::application::annual::AnnualResult present(const core::application::annual::AnnualResult& result)
{
    return presenter().present(result);
}

QVariantMap toPayload(const core::application::annual::AnnualResult& result)
{
    const auto projected = present(result);

    QVariantMap out;
    out.insert(QStringLiteral("annualId"), QString::fromStdString(projected.annualId));
    out.insert(QStringLiteral("annualName"), QString::fromStdString(projected.annualName));
    out.insert(QStringLiteral("year"), projected.year);

    QVariantMap stats;
    stats.insert(QStringLiteral("assignedAnalysisCount"), projected.stats.assignedAnalysisCount);
    stats.insert(QStringLiteral("snapshotTransactionCount"), projected.stats.snapshotTransactionCount);
    stats.insert(QStringLiteral("missingFromYear"), projected.stats.missingFromYear);
    stats.insert(QStringLiteral("mixedYear"), projected.stats.mixedYear);
    stats.insert(QStringLiteral("duplicateCount"), projected.stats.duplicateCount);
    stats.insert(QStringLiteral("missingLive"), projected.stats.missingLive);
    stats.insert(QStringLiteral("neutral"), projected.stats.neutral);
    stats.insert(QStringLiteral("unverified"), projected.stats.unverified);
    stats.insert(QStringLiteral("verified"), projected.stats.verified);
    stats.insert(QStringLiteral("completed"), projected.stats.completed);
    out.insert(QStringLiteral("stats"), stats);

    const QVariantList deduplicated = toRows(projected.deduplicated);
    const QVariantList similar = toRows(projected.similar);
    const QVariantList divergent = toRows(projected.divergent);
    const QVariantList workspaceOnly = toRows(projected.workspaceOnly);

    out.insert(QStringLiteral("deduplicated"), deduplicated);
    out.insert(QStringLiteral("similar"), similar);
    out.insert(QStringLiteral("divergent"), divergent);
    out.insert(QStringLiteral("workspaceOnly"), workspaceOnly);

    QVariantList all;
    all.reserve(deduplicated.size() + similar.size() + divergent.size() + workspaceOnly.size());
    for (const auto& value : deduplicated) all.push_back(value);
    for (const auto& value : similar) all.push_back(value);
    for (const auto& value : divergent) all.push_back(value);
    for (const auto& value : workspaceOnly) all.push_back(value);
    out.insert(QStringLiteral("transactions"), all);
    return out;
}

} // namespace ui::annual
