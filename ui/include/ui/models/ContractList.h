#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <vector>
#include <memory>

#include "core/models/Contract.h"

namespace ui {

class ContractList : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TypeRole,
        DescriptionRole,
        StartDateRole,
        EndDateRole,
        BasePriceRole,
        ConsumptionPriceRole,
        MonthlyAdvanceRole,
        ActorIdsRole,
        PropertyIdsRole
    };

    explicit ContractList(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setContracts(std::vector<std::shared_ptr<Contract>> contracts);
    const std::vector<std::shared_ptr<Contract>>& contracts() const;

    Q_INVOKABLE int addContract(const QString& name, const QString& type, const QString& description);
    Q_INVOKABLE void removeAt(int row);

private:
    std::vector<std::shared_ptr<Contract>> contracts_;

    static QStringList toQStringList(const std::vector<std::string>& v);
    static std::vector<std::string> toStdVectorStrings(const QVariant& v);
};

}
