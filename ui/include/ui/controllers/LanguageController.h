/**
 * @file ui/include/ui/controllers/LanguageController.h
 * @brief Declares the UI controller responsible for runtime language switching.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QTranslator>

class QApplication;
class QQmlEngine;

namespace ui {

/**
 * @brief Manages the active UI language and the available translation options.
 */
class LanguageController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE setCurrentLanguage NOTIFY currentLanguageChanged)
    Q_PROPERTY(QVariantList availableLanguages READ availableLanguages CONSTANT)

public:
    explicit LanguageController(QApplication* application, QQmlEngine* engine, QObject* parent = nullptr);

    QString currentLanguage() const { return currentLanguage_; }
    void setCurrentLanguage(const QString& languageCode);
    QVariantList availableLanguages() const { return availableLanguages_; }

signals:
    void currentLanguageChanged();

private:
    bool isLanguageAvailable(const QString& languageCode) const;
    QString normalizeLanguageCode(const QString& languageCode) const;
    QString translationFileName(const QString& languageCode) const;
    bool translationFileExists(const QString& languageCode) const;
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
