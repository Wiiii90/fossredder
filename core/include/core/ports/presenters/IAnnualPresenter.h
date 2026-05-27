#pragma once

#include "core/application/annual/AnnualResult.h"

namespace core::ports::presenters {

class IAnnualPresenter {
public:
    virtual ~IAnnualPresenter() = default;

    virtual core::application::annual::AnnualResult present(const core::application::annual::AnnualResult& result) const = 0;
};

} // namespace core::ports::presenters

