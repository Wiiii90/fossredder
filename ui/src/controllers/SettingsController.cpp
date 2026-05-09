/**
 * @file ui/src/controllers/SettingsController.cpp
 * @brief Implements the centralized UI settings controller exposed to QML.
 */

#include "ui/controllers/SettingsController.h"

#include <QSettings>

#include "core/constants/import.h"
#include "core/constants/localization.h"
#include "core/constants/preferences.h"

namespace ui {

namespace {

QString fromCoreString(std::string_view value)
{
    return QString::fromLatin1(value.data());
}

QSettings openSettingsStore()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     fromCoreString(core::constants::preferences::kOrganizationName),
                     fromCoreString(core::constants::preferences::kApplicationName));
}

}

SettingsController::SettingsController(QObject* parent)
    : QObject(parent)
{
    load();
}

QString SettingsController::normalizeText(const QString& value)
{
    return value.trimmed();
}

int SettingsController::normalizeArchiveFormat(int value) noexcept
{
    return value == 1 ? 1 : 0;
}

void SettingsController::emitStateChanged()
{
    emit stateChanged();
}

void SettingsController::applyDefaults()
{
    language_ = fromCoreString(core::constants::localization::languages::kEnglish);
    importDefaultPath_.clear();
    importPoppler_.clear();
    importOpenCv_.clear();
    importTesseract_.clear();
    importParser_.clear();
    importMatcher_.clear();
    exportDefaultDirectory_.clear();
    exportArchiveFormat_ = 0;
    exportIncludeFormulas_ = true;
    toolbarShowBooking_ = true;
    toolbarShowActors_ = true;
    toolbarShowProperties_ = true;
    toolbarShowContracts_ = true;
    toolbarShowImport_ = true;
    toolbarShowExport_ = true;
    toolbarShowAnalysis_ = true;
    toolbarShowAnnual_ = true;
    toolbarShowSettings_ = true;
}

void SettingsController::loadFromPersistentStore()
{
    auto settings = openSettingsStore();
    language_ = normalizeText(settings.value(fromCoreString(core::constants::preferences::keys::kLanguage), fromCoreString(core::constants::localization::languages::kEnglish)).toString());
    if (language_.isEmpty()) language_ = fromCoreString(core::constants::localization::languages::kEnglish);
    importDefaultPath_ = normalizeText(settings.value(fromCoreString(core::constants::preferences::keys::kImportDefaultPath)).toString());
    importPoppler_ = settings.value(fromCoreString(core::constants::preferences::keys::kImportPoppler)).toString();
    importOpenCv_ = settings.value(fromCoreString(core::constants::preferences::keys::kImportOpenCv)).toString();
    importTesseract_ = settings.value(fromCoreString(core::constants::preferences::keys::kImportTesseract)).toString();
    importParser_ = settings.value(fromCoreString(core::constants::preferences::keys::kImportParser)).toString();
    importMatcher_ = settings.value(fromCoreString(core::constants::preferences::keys::kImportMatcher)).toString();
    exportDefaultDirectory_ = normalizeText(settings.value(fromCoreString(core::constants::preferences::keys::kExportDefaultDirectory)).toString());
    exportArchiveFormat_ = normalizeArchiveFormat(settings.value(fromCoreString(core::constants::preferences::keys::kExportArchiveFormat), 0).toInt());
    exportIncludeFormulas_ = settings.value(fromCoreString(core::constants::preferences::keys::kExportIncludeFormulas), true).toBool();
    toolbarShowBooking_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowBooking), true).toBool();
    toolbarShowActors_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowActors), true).toBool();
    toolbarShowProperties_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowProperties), true).toBool();
    toolbarShowContracts_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowContracts), true).toBool();
    toolbarShowImport_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowImport), true).toBool();
    toolbarShowExport_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowExport), true).toBool();
    toolbarShowAnalysis_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowAnalysis), true).toBool();
    toolbarShowAnnual_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowAnnual), true).toBool();
    toolbarShowSettings_ = settings.value(fromCoreString(core::constants::preferences::keys::kToolbarShowSettings), true).toBool();
}

void SettingsController::persistToStore() const
{
    auto settings = openSettingsStore();
    settings.setValue(fromCoreString(core::constants::preferences::keys::kLanguage), language_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportDefaultPath), importDefaultPath_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportPoppler), importPoppler_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportOpenCv), importOpenCv_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportTesseract), importTesseract_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportParser), importParser_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kImportMatcher), importMatcher_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kExportDefaultDirectory), exportDefaultDirectory_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kExportArchiveFormat), exportArchiveFormat_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kExportIncludeFormulas), exportIncludeFormulas_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowBooking), toolbarShowBooking_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowActors), toolbarShowActors_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowProperties), toolbarShowProperties_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowContracts), toolbarShowContracts_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowImport), toolbarShowImport_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowExport), toolbarShowExport_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowAnalysis), toolbarShowAnalysis_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowAnnual), toolbarShowAnnual_);
    settings.setValue(fromCoreString(core::constants::preferences::keys::kToolbarShowSettings), toolbarShowSettings_);
    settings.sync();
}

void SettingsController::setLanguage(const QString& value)
{
    const QString normalized = normalizeText(value).toLower();
    const QString nextValue = normalized.isEmpty() ? fromCoreString(core::constants::localization::languages::kEnglish) : normalized;
    if (language_ == nextValue) return;
    language_ = nextValue;
    emit languageChanged();
    emitStateChanged();
}

void SettingsController::setImportDefaultPath(const QString& value)
{
    const QString nextValue = normalizeText(value);
    if (importDefaultPath_ == nextValue) return;
    importDefaultPath_ = nextValue;
    emit importDefaultPathChanged();
    emitStateChanged();
}

void SettingsController::setImportPoppler(const QString& value)
{
    if (importPoppler_ == value) return;
    importPoppler_ = value;
    emit importPopplerChanged();
    emitStateChanged();
}

void SettingsController::setImportOpenCv(const QString& value)
{
    if (importOpenCv_ == value) return;
    importOpenCv_ = value;
    emit importOpenCvChanged();
    emitStateChanged();
}

void SettingsController::setImportTesseract(const QString& value)
{
    if (importTesseract_ == value) return;
    importTesseract_ = value;
    emit importTesseractChanged();
    emitStateChanged();
}

void SettingsController::setImportParser(const QString& value)
{
    if (importParser_ == value) return;
    importParser_ = value;
    emit importParserChanged();
    emitStateChanged();
}

void SettingsController::setImportMatcher(const QString& value)
{
    if (importMatcher_ == value) return;
    importMatcher_ = value;
    emit importMatcherChanged();
    emitStateChanged();
}

void SettingsController::setExportDefaultDirectory(const QString& value)
{
    const QString nextValue = normalizeText(value);
    if (exportDefaultDirectory_ == nextValue) return;
    exportDefaultDirectory_ = nextValue;
    emit exportDefaultDirectoryChanged();
    emitStateChanged();
}

void SettingsController::setExportArchiveFormat(int value)
{
    const int nextValue = normalizeArchiveFormat(value);
    if (exportArchiveFormat_ == nextValue) return;
    exportArchiveFormat_ = nextValue;
    emit exportArchiveFormatChanged();
    emitStateChanged();
}

void SettingsController::setExportIncludeFormulas(bool value)
{
    if (exportIncludeFormulas_ == value) return;
    exportIncludeFormulas_ = value;
    emit exportIncludeFormulasChanged();
    emitStateChanged();
}

void SettingsController::setToolbarShowBooking(bool value)
{
    if (toolbarShowBooking_ == value) return;
    toolbarShowBooking_ = value;
    emit toolbarShowBookingChanged();
    emitStateChanged();
}

void SettingsController::setToolbarShowActors(bool value)
{
    if (toolbarShowActors_ == value) return;
    toolbarShowActors_ = value;
    emit toolbarShowActorsChanged();
    emitStateChanged();
}

void SettingsController::setToolbarShowProperties(bool value)
{
    if (toolbarShowProperties_ == value) return;
    toolbarShowProperties_ = value;
    emit toolbarShowPropertiesChanged();
    emitStateChanged();
}

void SettingsController::setToolbarShowContracts(bool value)
{
    if (toolbarShowContracts_ == value) return;
    toolbarShowContracts_ = value;
    emit toolbarShowContractsChanged();
    emitStateChanged();
}

void SettingsController::setToolbarShowImport(bool value)
{
    if (toolbarShowImport_ == value) return;
    toolbarShowImport_ = value;
    emit toolbarShowImportChanged();
    emitStateChanged();
}

void SettingsController::setToolbarShowExport(bool value)
{
    if (toolbarShowExport_ == value) return;
    toolbarShowExport_ = value;
    emit toolbarShowExportChanged();
    emitStateChanged();
}

void SettingsController::setToolbarShowAnalysis(bool value)
{
    if (toolbarShowAnalysis_ == value) return;
    toolbarShowAnalysis_ = value;
    emit toolbarShowAnalysisChanged();
    emitStateChanged();
}

void SettingsController::setToolbarShowAnnual(bool value)
{
    if (toolbarShowAnnual_ == value) return;
    toolbarShowAnnual_ = value;
    emit toolbarShowAnnualChanged();
    emitStateChanged();
}

void SettingsController::setToolbarShowSettings(bool value)
{
    if (toolbarShowSettings_ == value) return;
    toolbarShowSettings_ = value;
    emit toolbarShowSettingsChanged();
    emitStateChanged();
}

void SettingsController::load()
{
    applyDefaults();
    loadFromPersistentStore();

    savedLanguage_ = language_;
    savedImportDefaultPath_ = importDefaultPath_;
    savedImportPoppler_ = importPoppler_;
    savedImportOpenCv_ = importOpenCv_;
    savedImportTesseract_ = importTesseract_;
    savedImportParser_ = importParser_;
    savedImportMatcher_ = importMatcher_;
    savedExportDefaultDirectory_ = exportDefaultDirectory_;
    savedExportArchiveFormat_ = exportArchiveFormat_;
    savedExportIncludeFormulas_ = exportIncludeFormulas_;
    savedToolbarShowBooking_ = toolbarShowBooking_;
    savedToolbarShowActors_ = toolbarShowActors_;
    savedToolbarShowProperties_ = toolbarShowProperties_;
    savedToolbarShowContracts_ = toolbarShowContracts_;
    savedToolbarShowImport_ = toolbarShowImport_;
    savedToolbarShowExport_ = toolbarShowExport_;
    savedToolbarShowAnalysis_ = toolbarShowAnalysis_;
    savedToolbarShowAnnual_ = toolbarShowAnnual_;
    savedToolbarShowSettings_ = toolbarShowSettings_;

    emit stateChanged();
}

void SettingsController::save()
{
    persistToStore();
    savedLanguage_ = language_;
    savedImportDefaultPath_ = importDefaultPath_;
    savedImportPoppler_ = importPoppler_;
    savedImportOpenCv_ = importOpenCv_;
    savedImportTesseract_ = importTesseract_;
    savedImportParser_ = importParser_;
    savedImportMatcher_ = importMatcher_;
    savedExportDefaultDirectory_ = exportDefaultDirectory_;
    savedExportArchiveFormat_ = exportArchiveFormat_;
    savedExportIncludeFormulas_ = exportIncludeFormulas_;
    savedToolbarShowBooking_ = toolbarShowBooking_;
    savedToolbarShowActors_ = toolbarShowActors_;
    savedToolbarShowProperties_ = toolbarShowProperties_;
    savedToolbarShowContracts_ = toolbarShowContracts_;
    savedToolbarShowImport_ = toolbarShowImport_;
    savedToolbarShowExport_ = toolbarShowExport_;
    savedToolbarShowAnalysis_ = toolbarShowAnalysis_;
    savedToolbarShowAnnual_ = toolbarShowAnnual_;
    savedToolbarShowSettings_ = toolbarShowSettings_;
    emit saved();
    emit stateChanged();
}

void SettingsController::resetToDefaults()
{
    applyDefaults();
    emit languageChanged();
    emit importDefaultPathChanged();
    emit importPopplerChanged();
    emit importOpenCvChanged();
    emit importTesseractChanged();
    emit importParserChanged();
    emit importMatcherChanged();
    emit exportDefaultDirectoryChanged();
    emit exportArchiveFormatChanged();
    emit exportIncludeFormulasChanged();
    emit toolbarShowBookingChanged();
    emit toolbarShowActorsChanged();
    emit toolbarShowPropertiesChanged();
    emit toolbarShowContractsChanged();
    emit toolbarShowImportChanged();
    emit toolbarShowExportChanged();
    emit toolbarShowAnalysisChanged();
    emit toolbarShowAnnualChanged();
    emit toolbarShowSettingsChanged();
    emit reset();
    emitStateChanged();
}

bool SettingsController::hasChanges() const noexcept
{
    return language_ != savedLanguage_
        || importDefaultPath_ != savedImportDefaultPath_
        || importPoppler_ != savedImportPoppler_
        || importOpenCv_ != savedImportOpenCv_
        || importTesseract_ != savedImportTesseract_
        || importParser_ != savedImportParser_
        || importMatcher_ != savedImportMatcher_
        || exportDefaultDirectory_ != savedExportDefaultDirectory_
        || exportArchiveFormat_ != savedExportArchiveFormat_
        || exportIncludeFormulas_ != savedExportIncludeFormulas_
        || toolbarShowBooking_ != savedToolbarShowBooking_
        || toolbarShowActors_ != savedToolbarShowActors_
        || toolbarShowProperties_ != savedToolbarShowProperties_
        || toolbarShowContracts_ != savedToolbarShowContracts_
        || toolbarShowImport_ != savedToolbarShowImport_
        || toolbarShowExport_ != savedToolbarShowExport_
        || toolbarShowAnalysis_ != savedToolbarShowAnalysis_
        || toolbarShowAnnual_ != savedToolbarShowAnnual_
        || toolbarShowSettings_ != savedToolbarShowSettings_;
}

} // namespace ui
