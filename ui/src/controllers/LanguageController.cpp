#include "ui/controllers/LanguageController.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QLocale>
#include <QQmlEngine>
#include <QSettings>

#include "ui/config/Defaults.h"

namespace ui {

namespace {

QVariantMap makeLanguageOption(const char* code, const char* label, bool available)
{
    QVariantMap option;
    option.insert(QStringLiteral("code"), QString::fromLatin1(code));
    option.insert(QStringLiteral("label"), QString::fromLatin1(label));
    option.insert(QStringLiteral("available"), available);
    return option;
}

}

LanguageController::LanguageController(QApplication* application, QQmlEngine* engine, QObject* parent)
    : QObject(parent)
    , application_(application)
    , engine_(engine)
{
    availableLanguages_.append(makeLanguageOption(ui::config::kLanguageEnglishCode, "English", true));
    availableLanguages_.append(makeLanguageOption(ui::config::kLanguageGermanCode,
                                                  "Deutsch",
                                                  translationFileExists(ui::config::kLanguageGermanCode)));

    const QString preferredLanguage = normalizeLanguageCode(persistedLanguage());
    setCurrentLanguage(preferredLanguage);
}

void LanguageController::setCurrentLanguage(const QString& languageCode)
{
    const QString normalizedLanguage = normalizeLanguageCode(languageCode);
    QString targetLanguage = normalizedLanguage;
    if (!isLanguageAvailable(targetLanguage)) targetLanguage = QString::fromLatin1(ui::config::kLanguageEnglishCode);
    if (currentLanguage_ == targetLanguage) return;

    if (application_) application_->removeTranslator(&translator_);

    if (targetLanguage != QLatin1String(ui::config::kLanguageEnglishCode)) {
        if (!loadTranslation(targetLanguage)) {
            targetLanguage = QString::fromLatin1(ui::config::kLanguageEnglishCode);
        }
    }

    currentLanguage_ = targetLanguage;
    persistLanguage(currentLanguage_);
    retranslateUi();
    emit currentLanguageChanged();
}

bool LanguageController::isLanguageAvailable(const QString& languageCode) const
{
    const QString normalizedLanguage = normalizeLanguageCode(languageCode);
    if (normalizedLanguage == QLatin1String(ui::config::kLanguageEnglishCode)) return true;
    return translationFileExists(normalizedLanguage);
}

QString LanguageController::normalizeLanguageCode(const QString& languageCode) const
{
    const QString trimmed = languageCode.trimmed().toLower();
    if (trimmed.startsWith(QLatin1String(ui::config::kLanguageGermanCode))) {
        return QString::fromLatin1(ui::config::kLanguageGermanCode);
    }
    return QString::fromLatin1(ui::config::kLanguageEnglishCode);
}

QString LanguageController::translationFileName(const QString& languageCode) const
{
    return QStringLiteral("%1_%2.qm").arg(ui::config::kTranslationBaseName, languageCode);
}

bool LanguageController::translationFileExists(const QString& languageCode) const
{
    if (languageCode == QLatin1String(ui::config::kLanguageEnglishCode)) return true;

    const QString fileName = translationFileName(languageCode);
    const QString appTranslationPath = QCoreApplication::applicationDirPath() + QLatin1Char('/') + ui::config::kTranslationsDirName + QLatin1Char('/') + fileName;
    if (QFileInfo::exists(appTranslationPath)) return true;

    const QString resourceTranslationPath = QStringLiteral(":/i18n/") + fileName;
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

    if (translator_.load(QStringLiteral(":/i18n/") + fileName)) {
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
    QSettings settings;
    settings.setValue(ui::config::kLanguageSettingsKey, languageCode);
}

QString LanguageController::persistedLanguage() const
{
    QSettings settings;
    const QString fallbackLanguage = normalizeLanguageCode(QLocale::system().name());
    return settings.value(ui::config::kLanguageSettingsKey, fallbackLanguage).toString();
}

}
