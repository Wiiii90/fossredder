/**
 * @file ui/include/ui/controllers/LanguageController.h
 * @brief Declares the UI controller responsible for runtime language switching.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QTranslator>
#include <qqmlintegration.h>

class QApplication;
class QQmlEngine;

namespace ui {

/**
 * @brief Manages the active UI language and the available translation options.
 */
class LanguageController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(LanguageController)
    QML_UNCREATABLE("LanguageController is provided by the application context")
    Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE setCurrentLanguage NOTIFY currentLanguageChanged)
    Q_PROPERTY(QVariantList availableLanguages READ availableLanguages CONSTANT)

public:
    /** @brief Create a language controller bound to the application and QML engine. */
    explicit LanguageController(QApplication* application, QQmlEngine* engine, QObject* parent = nullptr);

    /** @brief Return the active UI language code.
     *  @return Current language code
     */
    QString currentLanguage() const { return currentLanguage_; }

    /** @brief Switch the active UI language if the requested translation is available.
     *  @param languageCode Language code to set
     */
    void setCurrentLanguage(const QString& languageCode);

    /** @brief Return the available language options for the UI. */
    QVariantList availableLanguages() const { return availableLanguages_; }

    Q_INVOKABLE bool applyLanguage(const QString& languageCode);

signals:
    void currentLanguageChanged();

private:
    void refreshAvailableLanguages();
    bool isLanguageAvailable(const QString& languageCode) const;
    QString normalizeLanguageCode(const QString& languageCode) const;
    QString translationFileName(const QString& languageCode) const;
    bool translationFileExists(const QString& languageCode) const;
    bool applyCurrentLanguage(const QString& languageCode);
    bool loadTranslation(const QString& languageCode);
    void retranslateUi();
    void persistLanguage(const QString& languageCode);
    QString persistedLanguage() const;

    QApplication* application_ = nullptr;
    QQmlEngine* engine_ = nullptr;
    QTranslator translator_;
    QVariantList availableLanguages_;
    QString currentLanguage_;
};

} // namespace ui
