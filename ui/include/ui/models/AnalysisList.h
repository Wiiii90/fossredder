#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>
#include <memory>

#include "core/models/Analysis.h"

namespace ui {

class AnalysisList : public QAbstractListModel {
    Q_OBJECT
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

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setAnalyses(std::vector<std::shared_ptr<Analysis>> analyses);
    std::vector<std::shared_ptr<Analysis>> analyses() const;

    Q_INVOKABLE int addAnalysis(const QString& name, const QString& type);
    Q_INVOKABLE void removeAt(int row);
    Q_INVOKABLE bool updateAnalysisById(const QString& id, const QString& name, const QString& type, const QString& configJson, const QString& filterSpec);
    Q_INVOKABLE void setAdjustmentsById(const QString& id, const QString& json);

private:
    std::vector<std::shared_ptr<Analysis>> analyses_;
};

}
