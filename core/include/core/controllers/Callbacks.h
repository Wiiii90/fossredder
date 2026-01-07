#pragma once

/**
 * @file core/include/core/controllers/Callbacks.h
 * @brief Collection of controller callback type aliases.
 *
 * Central header that collects callback type aliases used by controllers,
 * e.g. callbacks for persistence impact notifications.
 *
 * @note Header-only, no implementation file required.
 */

#include <functional>

#include "core/models/DeletionImpact.h"

/**
 * @brief Callback invoked after a persistence operation to report deleted IDs.
 *
 * The callback receives a constant reference to a `DeletionImpact` containing
 * the IDs of deleted actors, properties, contracts, statements and transactions.
 *
 * @see core::models::DeletionImpact
 */
using DeletionImpactCallback = std::function<void(const DeletionImpact&)>;