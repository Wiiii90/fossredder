#pragma once

#include <functional>

#include "core/models/DeletionImpact.h"

using DeletionImpactCallback = std::function<void(const DeletionImpact&)>;
