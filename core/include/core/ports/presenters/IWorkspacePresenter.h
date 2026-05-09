#pragma once

#include <string>

namespace core::ports::presenters {

struct WorkspacePresentation {
    std::string currentPath;
    bool hasCurrentPath = false;
};

class IWorkspacePresenter {
public:
    virtual ~IWorkspacePresenter() = default;

    virtual WorkspacePresentation present(const WorkspacePresentation& result) const = 0;
};

} // namespace core::ports::presenters
