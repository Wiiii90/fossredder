/**
 * @file ui/include/ui/controllers/AnnualController.h
 * @brief Declares the UI controller that forwards annual aggregate creation to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

namespace core::application { class AppStateFacade; }

namespace ui {

/**
 * @brief Exposes annual aggregate creation to QML.
 */
class AnnualController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(AnnualController)
    QML_UNCREATABLE("AnnualController is provided by the application context")
public:
    /** @brief Create an annual controller bound to the application facade.
     *  @param core Core application facade pointer
     *  @param parent QObject parent
     */
    explicit AnnualController(core::application::AppStateFacade* core, QObject* parent = nullptr);

    /** @brief Return a single annual aggregate by identifier.
     *  @param id Annual identifier
     *  @return Serialized annual payload as QVariantMap
     */
    Q_INVOKABLE QVariantMap annual(const QString& id) const;

    /** @brief Return all annual aggregates.
     *  @return List of serialized annual aggregates as QVariantList
     */
    Q_INVOKABLE QVariantList annuals() const;

    /** @brief Create an annual aggregate for the specified year.
     *  @param year Year for the aggregate
     *  @return Identifier of the created annual aggregate
     */
    Q_INVOKABLE QString addAnnual(int year);

private:
    core::application::AppStateFacade* core_ = nullptr;
};

} // namespace ui
