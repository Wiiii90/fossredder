/**
 * @file ui/include/ui/state/SessionMutationState.h
 * @brief Declarations for the UI SessionMutationState component.
 */

#pragma once

#include <functional>

#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QStringList>

#include "core/models/DeletionImpact.h"
#include "core/models/Transaction.h"

namespace ui {

class SessionModels;
class FilterState;

class SessionMutationState {
public:
    static QVariantList normalizeStrings(const QVariantList& values);
    static QVariantList addUniqueTrimmed(const QVariantList& values, const QString& value);
    static QVariantList removeAt(const QVariantList& values, int index);
    static QVariantList removeString(const QVariantList& values, const QString& value);
    static QVariantList insertAt(const QVariantList& values, int index, const QVariant& value);

    static void applyDeletionImpact(const core::domain::DeletionImpact& impact,
                                    SessionModels& models,
                                    FilterState& filters);

    static void setTransactionPropertyIdsImmediate(const QString& transactionId,
                                                   const QStringList& propertyIds,
                                                    SessionModels& models);
};

}
