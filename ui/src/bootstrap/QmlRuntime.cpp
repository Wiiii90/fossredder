#include "ui/bootstrap/QmlRuntime.h"

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QQmlEngine>

#include "ui/bootstrap/QmlContracts.h"
#include "ui/config/Defaults.h"
#include "ui/state/NavigationState.h"

namespace ui::bootstrap {

void registerTypes()
{
    static bool registered = false;
    if (registered) return;

    qmlRegisterUncreatableType<ui::NavigationState>(ui::qml::contracts::module::kName,
                                                    ui::qml::contracts::module::kMajorVersion,
                                                    ui::qml::contracts::module::kMinorVersion,
                                                    ui::qml::contracts::module::kNavigationTypeName,
                                                    ui::qml::contracts::module::kNavigationTypeDescription);
    registered = true;
}

void configureRuntime(QQmlEngine* engine)
{
    if (!engine) return;

    const QString qtImports = QLibraryInfo::path(QLibraryInfo::Qml2ImportsPath);
    if (!qtImports.isEmpty() && QDir(qtImports).exists()) {
        engine->addImportPath(qtImports);
    }

    const QString appQmlDir = QCoreApplication::applicationDirPath() + QLatin1Char('/') + ui::config::kAppQmlDirName;
    if (QDir(appQmlDir).exists()) {
        engine->addImportPath(appQmlDir);
    }

    const QString imageFormatsDir = QCoreApplication::applicationDirPath() + QLatin1Char('/') + ui::config::kImageFormatsDirName;
    if (QDir(imageFormatsDir).exists()) {
        QCoreApplication::addLibraryPath(imageFormatsDir);
    }

    engine->addImportPath(ui::config::kQrcQmlImportPath);
}

}
