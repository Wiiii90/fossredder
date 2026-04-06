#pragma once

#include "core/models/Analysis.h"
#include "ui/models/IndexedListModel.h"

namespace ui {

class AnalysisList : public models::IndexedListModel<core::domain::Analysis> {
    Q_OBJECT
    using Base = models::IndexedListModel<core::domain::Analysis>;
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        ConfigRole,
        FilterRole,
        AdjustmentsRole
    };

    explicit AnalysisList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setAnalyses(std::vector<std::shared_ptr<core::domain::Analysis>> analyses);
    const std::vector<std::shared_ptr<core::domain::Analysis>>& analyses() const { return items(); }
    int findRowById(const QString& id) const { return findIndexedRow(id); }

    Q_INVOKABLE int addAnalysis(const QString& name, const QString& type);
    Q_INVOKABLE void removeAt(int row);
    Q_INVOKABLE bool updateAnalysisById(const QString& id, const QString& name, const QString& type, const QString& configJson, const QString& filterSpec);
    Q_INVOKABLE void setAdjustmentsById(const QString& id, const QString& adjustmentsJson);

private:
    static QString serializeAdjustmentsJson(const core::domain::Analysis& analysis);
    void refreshAdjustmentsCache();
    void refreshAdjustmentsCacheEntry(const core::domain::Analysis& analysis);

    QHash<QString, QString> adjustmentsJsonById_;
};

}
