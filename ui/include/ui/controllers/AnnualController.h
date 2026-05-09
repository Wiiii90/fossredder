/**
 * @file ui/include/ui/controllers/AnnualController.h
 * @brief Declares the UI controller that forwards annual aggregate creation to the application facade.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

namespace core::application { class WorkspaceFacade; }

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
    explicit AnnualController(core::application::WorkspaceFacade* core, QObject* parent = nullptr);

    /** @brief Return a single annual aggregate by identifier.
     *  @param id Annual identifier
     *  @return Serialized annual payload as QVariantMap
     */
    Q_INVOKABLE QVariantMap annual(const QString& id) const;

    /** @brief Return all annual aggregates.
     *  @return List of serialized annual aggregates as QVariantList
     */
    Q_INVOKABLE QVariantList annuals() const;

    /** @brief Create an annual aggregate for the specified name and year.
     *  @param name Name for the aggregate
     *  @param year Year for the aggregate
     *  @return Identifier of the created annual aggregate
     */
    Q_INVOKABLE QString addAnnual(const QString& name,
                                  int year,
                                  const QStringList& analysisIds = {});

    /** @brief Update an existing annual aggregate.
     *  @param id Annual identifier
     *  @param name Name for the aggregate
     *  @param year Year for the aggregate
     */
    Q_INVOKABLE void updateAnnual(const QString& id,
                                  const QString& name,
                                  int year,
                                  const QStringList& analysisIds = {});

    /** @brief Create or update an annual aggregate based on identifier presence.
     *  @param id Annual identifier (empty to create)
     *  @param name Name for the aggregate
     *  @param year Year for the aggregate
     *  @return Identifier of the saved annual aggregate
     */
    Q_INVOKABLE QString saveAnnual(const QString& id,
                                   const QString& name,
                                   int year,
                                   const QStringList& analysisIds = {});

    /** @brief Delete an annual aggregate by identifier.
     *  @param id Annual identifier
     */
    Q_INVOKABLE void deleteAnnual(const QString& id);

private:
    core::application::WorkspaceFacade* core_ = nullptr;
};

} // namespace ui
