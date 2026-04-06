/**
 * @file ui/include/ui/controllers/TransactionController.h
 * @brief Declares the UI controller that forwards transaction mutations to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include "core/models/Transaction.h"

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes transaction create, update and delete actions to QML.
 */
class TransactionController : public QObject {
    Q_OBJECT
public:
    ~TransactionController() override = default;

    /** @brief Create a transaction controller bound to the application facade. */
    explicit TransactionController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    /** @brief Create a new transaction from QML-provided values. */
    Q_INVOKABLE QString addTransaction(const QString& name,
                                       const QString& bookingDate,
                                       double amount,
                                       const QString& description,
                                       const QString& statementId,
                                       int status = static_cast<int>(core::domain::Transaction::Status::Neutral),
                                       const QString& actorId = QString(),
                                       bool allocatable = false,
                                       const QStringList& propertyIds = {});

    /** @brief Update an existing transaction from QML-provided values. */
    Q_INVOKABLE void updateTransaction(const QString& id,
                                       const QString& name,
                                       const QString& bookingDate,
                                       double amount,
                                       const QString& description,
                                       const QString& statementId,
                                       int status,
                                       const QString& actorId,
                                       bool allocatable,
                                       const QStringList& propertyIds);

    /** @brief Delete a transaction by identifier. */
    Q_INVOKABLE void deleteTransaction(const QString& id);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
