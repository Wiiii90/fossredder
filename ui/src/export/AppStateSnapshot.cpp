#include "ui/export/AppStateSnapshot.h"

#include "ui/state/AppStateClone.h"

namespace ui::exporting {

std::shared_ptr<const AppState> createSnapshot(const AppState& state)
{
    return std::make_shared<AppState>(ui::cloneAppState(state));
}

}
