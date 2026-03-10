#pragma once

#include <QStringList>

class QEvent;

namespace ui::window {

struct DropEventOutcome {
    bool handled = false;
    bool accepted = false;
    QStringList files;
};

class DropController {
public:
    DropEventOutcome handle(QEvent* event) const;
};

}
