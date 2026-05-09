/**
 * @file ui/src/controllers/LanguageController.cpp
 * @brief Implements runtime language switching and translation loading for the UI.
 */

#include "ui/controllers/LanguageController.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QLocale>
#include <QQmlEngine>
#include <QSettings>

#include "core/constants/localization.h"
#include "core/constants/preferences.h"
#include "ui/config/Defaults.h"
#include "ui/payload/PayloadKeys.h"
#include "ui/text/Text.h"

namespace ui {

namespace {

/** @brief Converts an ASCII shared constant into a QString for Qt APIs. */
QString fromCoreString(std::string_view value)
{
    return QString::fromLatin1(value.data());
}

/** @brief Creates the payload for a single language option entry. */
QVariantMap makeLanguageOption(const char* code, const QString& label, bool available)
{
    QVariantMap option;
    option.insert(payload::keys::language::kCode, QString::fromLatin1(code));
    option.insert(payload::keys::language::kLabel, label);
    option.insert(payload::keys::language::kAvailable, available);
    return option;
}

/** @brief Opens the stable per-user settings store used by the language controller. */
QSettings openLanguageSettings()
{
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     fromCoreString(core::constants::preferences::kOrganizationName),
                     fromCoreString(core::constants::preferences::kApplicationName));
}

}

LanguageController::LanguageController(QApplication* application, QQmlEngine* engine, QObject* parent)
    : QObject(parent)
    , application_(application)
    , engine_(engine)
{
    refreshAvailableLanguages();

    const QString preferredLanguage = normalizeLanguageCode(persistedLanguage());
    applyCurrentLanguage(preferredLanguage);
}

void LanguageController::refreshAvailableLanguages()
{
    availableLanguages_.clear();
    availableLanguages_.append(makeLanguageOption(core::constants::localization::languages::kEnglish.data(),
                                                  ui::text::language::englishLabel(),
                                                  true));
    availableLanguages_.append(makeLanguageOption(core::constants::localization::languages::kGerman.data(),
                                                  ui::text::language::germanLabel(),
                                                  true));
}

bool LanguageController::applyLanguage(const QString& languageCode)
{
    const bool changed = applyCurrentLanguage(languageCode);
    return changed || normalizeLanguageCode(languageCode) == currentLanguage_;
}

void LanguageController::setCurrentLanguage(const QString& languageCode)
{
    applyCurrentLanguage(languageCode);
}

bool LanguageController::applyCurrentLanguage(const QString& languageCode)
{
    const QString normalizedLanguage = normalizeLanguageCode(languageCode);
    QString targetLanguage = normalizedLanguage;
    if (!isLanguageAvailable(targetLanguage)) {
        targetLanguage = fromCoreString(core::constants::localization::languages::kEnglish);
    }

    const bool isSameLanguage = currentLanguage_ == targetLanguage;
    if (isSameLanguage && targetLanguage == fromCoreString(core::constants::localization::languages::kEnglish)) {
        persistLanguage(targetLanguage);
        return false;
    }

    if (application_) application_->removeTranslator(&translator_);

    if (targetLanguage != fromCoreString(core::constants::localization::languages::kEnglish)) {
        if (!loadTranslation(targetLanguage)) {
            targetLanguage = fromCoreString(core::constants::localization::languages::kEnglish);
        }
    }

    if (isSameLanguage && currentLanguage_ == targetLanguage) {
        if (application_) application_->installTranslator(&translator_);
        persistLanguage(targetLanguage);
        retranslateUi();
        return false;
    }

    currentLanguage_ = targetLanguage;
    persistLanguage(currentLanguage_);
    retranslateUi();
    emit currentLanguageChanged();
    return true;
}

bool LanguageController::isLanguageAvailable(const QString& languageCode) const
{
    const QString normalizedLanguage = normalizeLanguageCode(languageCode);
    if (normalizedLanguage == fromCoreString(core::constants::localization::languages::kEnglish)) return true;
    return translationFileExists(normalizedLanguage);
}

QString LanguageController::normalizeLanguageCode(const QString& languageCode) const
{
    const QString trimmed = languageCode.trimmed().toLower();
    if (trimmed.startsWith(fromCoreString(core::constants::localization::languages::kGerman))) {
        return fromCoreString(core::constants::localization::languages::kGerman);
    }
    return fromCoreString(core::constants::localization::languages::kEnglish);
}

QString LanguageController::translationFileName(const QString& languageCode) const
{
    return QStringLiteral("%1_%2.qm").arg(ui::config::kTranslationBaseName, languageCode);
}

bool LanguageController::translationFileExists(const QString& languageCode) const
{
    if (languageCode == fromCoreString(core::constants::localization::languages::kEnglish)) return true;

    const QString fileName = translationFileName(languageCode);
    const QString appTranslationPath = QCoreApplication::applicationDirPath() + QLatin1Char('/') + ui::config::kTranslationsDirName + QLatin1Char('/') + fileName;
    if (QFileInfo::exists(appTranslationPath)) return true;

    const QString resourceTranslationPath = ui::config::kTranslationResourcePrefix + fileName;
    return QFileInfo::exists(resourceTranslationPath);
}

bool LanguageController::loadTranslation(const QString& languageCode)
{
    const QString fileName = translationFileName(languageCode);
    const QString appTranslationDir = QCoreApplication::applicationDirPath() + QLatin1Char('/') + ui::config::kTranslationsDirName;

    if (translator_.load(fileName, appTranslationDir)) {
        if (application_) application_->installTranslator(&translator_);
        return true;
    }

    if (translator_.load(ui::config::kTranslationResourcePrefix + fileName)) {
        if (application_) application_->installTranslator(&translator_);
        return true;
    }

    return false;
}

void LanguageController::retranslateUi()
{
    if (engine_) engine_->retranslate();
}

void LanguageController::persistLanguage(const QString& languageCode)
{
    auto settings = openLanguageSettings();
    settings.setValue(fromCoreString(core::constants::preferences::keys::kLanguage), languageCode);
    settings.sync();
}

QString LanguageController::persistedLanguage() const
{
    const QString fallbackLanguage = normalizeLanguageCode(QLocale::system().name());
    auto settings = openLanguageSettings();
    return normalizeLanguageCode(
        settings.value(fromCoreString(core::constants::preferences::keys::kLanguage), fallbackLanguage).toString());
}

}
