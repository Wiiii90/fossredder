#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QTranslator>

class QApplication;
class QQmlEngine;

namespace ui {

class LanguageController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentLanguage READ currentLanguage WRITE setCurrentLanguage NOTIFY currentLanguageChanged)
    Q_PROPERTY(QVariantList availableLanguages READ availableLanguages CONSTANT)

public:
    explicit LanguageController(QApplication* application, QQmlEngine* engine, QObject* parent = nullptr);

    QString currentLanguage() const { return currentLanguage_; }
    void setCurrentLanguage(const QString& languageCode);
    QVariantList availableLanguages() const { return availableLanguages_; }

    Q_INVOKABLE bool isLanguageAvailable(const QString& languageCode) const;

signals:
    void currentLanguageChanged();

private:
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

}
