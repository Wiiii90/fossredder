#pragma once

class QQmlEngine;

namespace ui::bootstrap {

void registerTypes();
void configureRuntime(QQmlEngine* engine);

}
