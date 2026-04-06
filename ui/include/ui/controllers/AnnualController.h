/**
 * @file ui/include/ui/controllers/AnnualController.h
 * @brief Declares the UI controller that forwards annual aggregate creation to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes annual aggregate creation to QML.
 */
class AnnualController : public QObject {
    Q_OBJECT
public:
    /** @brief Create an annual controller bound to the application facade. */
    explicit AnnualController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    /** @brief Create an annual aggregate for the specified year. */
    Q_INVOKABLE QString addAnnual(int year);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
