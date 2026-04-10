/**
 * @file ui/include/ui/controllers/TransactionController.h
 * @brief Declares the UI controller that forwards transaction mutations to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

#include "core/models/Transaction.h"

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes transaction create, update and delete actions to QML.
 */
class TransactionController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(TransactionController)
    QML_UNCREATABLE("TransactionController is provided by the application context")
public:
    ~TransactionController() override = default;

    /** @brief Create a transaction controller bound to the application facade. */
    explicit TransactionController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    /** @brief Return a single transaction by identifier.
     *  @param id Transaction identifier
     *  @return Serialized transaction payload as QVariantMap
     */
    Q_INVOKABLE QVariantMap transaction(const QString& id) const;

    /** @brief Return all transactions.
     *  @return List of serialized transactions as QVariantList
     */
    Q_INVOKABLE QVariantList transactions() const;

    /** @brief Create a new transaction from QML-provided values.
     *  @param name Transaction name
     *  @param bookingDate Booking date string
     *  @param amount Transaction amount
     *  @param description Transaction description
     *  @param statementId Associated statement identifier
     *  @param status Raw status integer
     *  @param actorId Optional actor identifier
     *  @param allocatable Allocatable flag
     *  @param propertyIds Associated property identifiers
     *  @return Identifier of the created transaction
     */
    Q_INVOKABLE QString addTransaction(const QString& name,
                                       const QString& bookingDate,
                                       double amount,
                                       const QString& description,
                                       const QString& statementId,
                                       int status = static_cast<int>(core::domain::Transaction::Status::Neutral),
                                       const QString& actorId = QString(),
                                       bool allocatable = false,
                                       const QStringList& propertyIds = {});

    /** @brief Update an existing transaction from QML-provided values.
     *  @param id Transaction identifier
     *  @param name Transaction name
     *  @param bookingDate Booking date string
     *  @param amount Transaction amount
     *  @param description Transaction description
     *  @param statementId Associated statement identifier
     *  @param status Raw status integer
     *  @param actorId Optional actor identifier
     *  @param allocatable Allocatable flag
     *  @param propertyIds Associated property identifiers
     */
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

    /** @brief Delete a transaction by identifier.
     *  @param id Transaction identifier
     */
    Q_INVOKABLE void deleteTransaction(const QString& id);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
