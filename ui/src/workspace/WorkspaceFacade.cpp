/**
 * @file ui/src/workspace/WorkspaceFacade.cpp
 * @brief Implements the UI workspace facade that combines workspace data and
 * selection state.
 */

#include "ui/workspace/WorkspaceFacade.h"

#include <exception>

#include "core/application/annual/AnnualService.h"
#include "core/application/workspace/WorkspaceFacade.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/adapters/core/AnnualRequestMapper.h"
#include "ui/adapters/core/AnnualResultMapper.h"
#include "ui/adapters/core/EntityPayloadMapper.h"
#include "ui/adapters/core/WorkspaceRowProjector.h"
#include "ui/shared/payload/PayloadMapper.h"
#include "ui/shared/util/StringConversions.h"
#include "ui/state/session/ActorState.h"
#include "ui/state/session/BookingState.h"
#include "ui/state/session/ContractState.h"
#include "ui/state/session/PropertyState.h"

#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>

namespace ui {

namespace {

std::vector<core::ports::workspace::AliasSnapshot>
aliasSnapshots(const QStringList &aliases) {
  std::vector<core::ports::workspace::AliasSnapshot> out;
  out.reserve(aliases.size());
  for (const auto &alias : aliases) {
    const auto text = strings::toStdString(alias);
    out.push_back({text, {}, text, 0, {}, {}, {}});
  }
  return out;
}

std::vector<std::string> stdStrings(const QStringList &values) {
  return strings::toStdList(values);
}

QString analysisIdFromVariant(const QVariant &value) {
  if (!value.isValid())
    return {};
  if (value.typeId() == QMetaType::QString)
    return value.toString().trimmed();

  const QVariantMap map = value.toMap();
  if (!map.isEmpty()) {
    const QString id = map.value(QStringLiteral("id")).toString().trimmed();
    if (!id.isEmpty())
      return id;
    return map.value(QStringLiteral("objectId")).toString().trimmed();
  }
  return value.toString().trimmed();
}

QVariantMap normalizeAnnualAnalysisRow(const QVariant &value) {
  const QVariantMap source = value.toMap();
  QVariantMap out;
  const QString id = analysisIdFromVariant(value);
  if (id.isEmpty())
    return out;

  const QString name = source.value(QStringLiteral("name")).toString();
  const QString display = source.value(QStringLiteral("display")).toString();
  const QString type = source.value(QStringLiteral("type")).toString();

  out.insert(QStringLiteral("id"), id);
  out.insert(QStringLiteral("name"), name);
  out.insert(QStringLiteral("display"),
             !display.isEmpty() ? display : (!name.isEmpty() ? name : id));
  out.insert(QStringLiteral("type"),
             !type.isEmpty() ? type : QStringLiteral("tab"));
  out.insert(QStringLiteral("config"),
             source.value(QStringLiteral("config"), QStringLiteral("{}")));
  out.insert(
      QStringLiteral("filter"),
      source.value(QStringLiteral("filter"),
                   source.value(QStringLiteral("filterSpec"), QString())));
  out.insert(QStringLiteral("exportFormat"),
             source.value(QStringLiteral("exportFormat"), QString()));
  out.insert(QStringLiteral("includeCalcAdjustments"),
             source.value(QStringLiteral("includeCalcAdjustments"), true));
  out.insert(QStringLiteral("exportState"),
             source.value(QStringLiteral("exportState"), QStringLiteral("{}")));
  out.insert(
      QStringLiteral("snapshotTransactions"),
      source.value(QStringLiteral("snapshotTransactions"),
                   source.value(QStringLiteral("snapshotTransactionsJson"),
                                QStringLiteral("[]"))));
  return out;
}

core::ports::workspace::ActorCommand
makeActorCommand(const QString &id, const QString &name,
                 const QStringList &aliases, const QStringList &contractIds) {
  core::ports::workspace::ActorCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.aliases = aliasSnapshots(aliases);
  command.contractIds = stdStrings(contractIds);
  return command;
}

core::ports::workspace::PropertyCommand
makePropertyCommand(const QString &id, const QString &name,
                    const QStringList &aliases,
                    const QStringList &contractIds) {
  core::ports::workspace::PropertyCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.aliases = aliasSnapshots(aliases);
  command.contractIds = stdStrings(contractIds);
  return command;
}

core::ports::workspace::ContractCommand
makeContractCommand(const QString &id, const QString &name, const QString &type,
                    const QString &allocatableMode, const QStringList &actorIds,
                    const QStringList &propertyIds,
                    const QStringList &aliases) {
  core::ports::workspace::ContractCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.type = strings::toStdString(type);
  command.allocatableMode = strings::toStdString(allocatableMode);
  command.actorIds = stdStrings(actorIds);
  command.propertyIds = stdStrings(propertyIds);
  command.aliases = aliasSnapshots(aliases);
  return command;
}

core::ports::workspace::TransactionCommand
makeTransactionCommand(const QString &id, const QString &name,
                       const QString &bookingDate, const QString &valuta,
                       double amount, const QString &statementId,
                       const QString &insertAfterTransactionId, int status,
                       const QString &actorId, const QString &contractId,
                       bool allocatable, const QStringList &propertyIds) {
  core::ports::workspace::TransactionCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.bookingDate = strings::toStdString(bookingDate);
  command.valuta = strings::toStdString(valuta);
  command.amount = amount;
  command.statementId = strings::toStdString(statementId);
  command.insertAfterTransactionId =
      strings::toStdString(insertAfterTransactionId);
  command.status = static_cast<core::domain::Transaction::Status>(status);
  command.actorId = strings::toStdString(actorId);
  command.contractId = strings::toStdString(contractId);
  command.allocatable = allocatable;
  command.propertyIds = stdStrings(propertyIds);
  return command;
}

core::ports::workspace::AnalysisCommand makeAnalysisCommand(
    const QString &id, const QString &name, const QString &type,
    const QString &configJson, const QString &filterSpec,
    const QString &exportFormat, bool includeCalculationAdjustments,
    const QString &exportStateJson, const QString &snapshotTransactionsJson,
    std::vector<std::pair<std::string, double>> adjustments = {}) {
  core::ports::workspace::AnalysisCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.type = strings::toStdString(type);
  command.configJson = strings::toStdString(configJson);
  command.filterSpec = strings::toStdString(filterSpec);
  command.exportFormat = strings::toStdString(exportFormat);
  command.includeCalculationAdjustments = includeCalculationAdjustments;
  command.exportStateJson = strings::toStdString(exportStateJson);
  command.snapshotTransactionsJson =
      strings::toStdString(snapshotTransactionsJson);
  command.adjustments = std::move(adjustments);
  return command;
}

std::vector<std::pair<std::string, double>>
analysisAdjustmentsFromJson(const QString &adjustmentsJson) {
  std::vector<std::pair<std::string, double>> out;
  const QJsonDocument document = QJsonDocument::fromJson(adjustmentsJson.toUtf8());
  if (!document.isObject()) {
    return out;
  }
  const QJsonObject object = document.object();
  out.reserve(static_cast<size_t>(object.size()));
  for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
    if (!it.value().isDouble()) {
      continue;
    }
    out.emplace_back(strings::toStdString(it.key()), it.value().toDouble());
  }
  return out;
}

core::ports::workspace::AnnualCommand
makeAnnualCommand(const QString &id, const QString &name, int year,
                  const QStringList &analysisIds) {
  core::ports::workspace::AnnualCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.year = year;
  command.assignedAnalysisIds = stdStrings(analysisIds);
  return command;
}

} // namespace

WorkspaceFacade::WorkspaceFacade(QObject *parent)
    : QObject(parent), session_(std::make_unique<SessionState>(this)),
      selection_(std::make_unique<SessionSelection>(session_->models(), this)),
      actorState_(std::make_unique<ActorState>(this, this)),
      bookingState_(std::make_unique<BookingState>(this, this)),
      propertyState_(std::make_unique<PropertyState>(this, this)),
      contractState_(std::make_unique<ContractState>(this, this)) {
  QObject::connect(selection_.get(), &SessionSelection::selectedActorIdChanged,
                   this, &WorkspaceFacade::selectedActorIdChanged);
  QObject::connect(selection_.get(),
                   &SessionSelection::selectedPropertyIdChanged, this,
                   &WorkspaceFacade::selectedPropertyIdChanged);
  QObject::connect(selection_.get(),
                   &SessionSelection::selectedContractIdChanged, this,
                   &WorkspaceFacade::selectedContractIdChanged);
  QObject::connect(selection_.get(),
                   &SessionSelection::selectedStatementIdChanged, this,
                   &WorkspaceFacade::selectedStatementIdChanged);
  QObject::connect(selection_.get(),
                   &SessionSelection::selectedTransactionIdChanged, this,
                   &WorkspaceFacade::selectedTransactionIdChanged);
  QObject::connect(selection_.get(),
                   &SessionSelection::selectedAnalysisIdChanged, this,
                   &WorkspaceFacade::selectedAnalysisIdChanged);
  QObject::connect(selection_.get(), &SessionSelection::selectedAnnualIdChanged,
                   this, &WorkspaceFacade::selectedAnnualIdChanged);
  QObject::connect(selection_.get(),
                   &SessionSelection::lastAnalysisResultChanged, this,
                   &WorkspaceFacade::lastAnalysisResultChanged);
}

WorkspaceFacade::WorkspaceFacade(core::application::WorkspaceFacade *coreFacade,
                                 QObject *parent)
    : WorkspaceFacade(parent) {
  setCoreFacade(coreFacade);
}

SessionState *WorkspaceFacade::session() noexcept { return session_.get(); }
SessionSelection *WorkspaceFacade::selection() noexcept {
  return selection_.get();
}
ActorState *WorkspaceFacade::actorState() noexcept { return actorState_.get(); }
BookingState *WorkspaceFacade::bookingState() noexcept {
  return bookingState_.get();
}
PropertyState *WorkspaceFacade::propertyState() noexcept {
  return propertyState_.get();
}
ContractState *WorkspaceFacade::contractState() noexcept {
  return contractState_.get();
}

void WorkspaceFacade::bumpDataRevision() {
  ++dataRevision_;
  emit dataRevisionChanged();
}

void WorkspaceFacade::setCoreFacade(
    core::application::WorkspaceFacade *coreFacade) noexcept {
  coreFacade_ = coreFacade;
  if (!coreFacade_)
    return;
  loadFromState(coreFacade_->state());
  coreFacade_->setSnapshotChangedCallback(
      [this](const core::ports::workspace::WorkspaceSnapshot &snapshot) {
        (void)snapshot;
        loadFromState(coreFacade_->state());
      });
}

core::application::WorkspaceFacade *
WorkspaceFacade::coreFacade() const noexcept {
  return coreFacade_;
}

void WorkspaceFacade::loadFromState(
    const core::domain::catalog::WorkspaceCatalog &state) {
  session_->loadFromState(state);
  selection_->loadFromState();
  bumpDataRevision();
}

void WorkspaceFacade::loadFromState(
    const core::application::workspace::WorkspaceSessionState &state) {
  loadFromState(state.catalog);
}

QString WorkspaceFacade::currentPath() const {
  return coreFacade_ ? QString::fromStdString(coreFacade_->currentPath())
                     : QString();
}

void WorkspaceFacade::newFile(const QString &path) {
  if (!coreFacade_)
    return;
  try {
    coreFacade_->newFile(strings::toEncodedPath(path));
    emit operationSucceeded(QStringLiteral("newFile"));
  } catch (const std::exception &ex) {
    emit operationFailed(QStringLiteral("newFile"),
                         QString::fromUtf8(ex.what()));
  }
}

void WorkspaceFacade::openFile(const QString &path) {
  if (!coreFacade_)
    return;
  try {
    coreFacade_->openFile(strings::toEncodedPath(path));
    emit operationSucceeded(QStringLiteral("openFile"));
  } catch (const std::exception &ex) {
    emit operationFailed(QStringLiteral("openFile"),
                         QString::fromUtf8(ex.what()));
  }
}

void WorkspaceFacade::saveFile() {
  if (!coreFacade_)
    return;
  try {
    coreFacade_->saveFile();
    emit operationSucceeded(QStringLiteral("saveFile"));
  } catch (const std::exception &ex) {
    emit operationFailed(QStringLiteral("saveFile"),
                         QString::fromUtf8(ex.what()));
  }
}

void WorkspaceFacade::saveFileAs(const QString &path) {
  if (!coreFacade_)
    return;
  try {
    coreFacade_->saveFileAs(strings::toEncodedPath(path));
    emit operationSucceeded(QStringLiteral("saveFileAs"));
  } catch (const std::exception &ex) {
    emit operationFailed(QStringLiteral("saveFileAs"),
                         QString::fromUtf8(ex.what()));
  }
}

ActorList *WorkspaceFacade::actors() noexcept {
  return &session_->models().actors();
}
PropertyList *WorkspaceFacade::properties() noexcept {
  return &session_->models().properties();
}
ContractList *WorkspaceFacade::contracts() noexcept {
  return &session_->models().contracts();
}
StatementList *WorkspaceFacade::statements() noexcept {
  return &session_->models().statements();
}
TransactionList *WorkspaceFacade::transactions() noexcept {
  return &session_->models().transactions();
}
AnalysisList *WorkspaceFacade::analyses() noexcept {
  return &session_->models().analyses();
}
AnnualList *WorkspaceFacade::annuals() noexcept {
  return &session_->models().annuals();
}

QString WorkspaceFacade::selectedActorId() const {
  return selection_->selectedActorId();
}
QString WorkspaceFacade::selectedPropertyId() const {
  return selection_->selectedPropertyId();
}
QString WorkspaceFacade::selectedContractId() const {
  return selection_->selectedContractId();
}
QString WorkspaceFacade::selectedStatementId() const {
  return selection_->selectedStatementId();
}
QString WorkspaceFacade::selectedTransactionId() const {
  return selection_->selectedTransactionId();
}
QString WorkspaceFacade::selectedAnalysisId() const {
  return selection_->selectedAnalysisId();
}
QString WorkspaceFacade::selectedAnnualId() const {
  return selection_->selectedAnnualId();
}

void WorkspaceFacade::setSelectedActorId(const QString &id) {
  selection_->setSelectedActorId(id);
}

void WorkspaceFacade::setSelectedPropertyId(const QString &id) {
  selection_->setSelectedPropertyId(id);
}

void WorkspaceFacade::setSelectedContractId(const QString &id) {
  selection_->setSelectedContractId(id);
}

void WorkspaceFacade::setSelectedStatementId(const QString &id) {
  selection_->setSelectedStatementId(id);
}

void WorkspaceFacade::setSelectedTransactionId(const QString &id) {
  selection_->setSelectedTransactionId(id);
}

void WorkspaceFacade::setSelectedAnalysisId(const QString &id) {
  selection_->setSelectedAnalysisId(id);
}

void WorkspaceFacade::setSelectedAnnualId(const QString &id) {
  selection_->setSelectedAnnualId(id);
}

ActorSelection *WorkspaceFacade::selectedActor() {
  return selection_->selectedActor();
}
PropertySelection *WorkspaceFacade::selectedProperty() {
  return selection_->selectedProperty();
}
ContractSelection *WorkspaceFacade::selectedContract() {
  return selection_->selectedContract();
}
StatementSelection *WorkspaceFacade::selectedStatement() {
  return selection_->selectedStatement();
}
TransactionSelection *WorkspaceFacade::selectedTransaction() {
  return selection_->selectedTransaction();
}
AnalysisSelection *WorkspaceFacade::selectedAnalysis() {
  return selection_->selectedAnalysis();
}
AnnualSelection *WorkspaceFacade::selectedAnnual() {
  return selection_->selectedAnnual();
}

QVariantList
WorkspaceFacade::statementTransactionIds(const QString &statementId) const {
  return buildStatementTransactionIds(*session_, statementId);
}

QVariantList WorkspaceFacade::contractRows() const {
  return buildContractRows(*session_);
}

QVariantList WorkspaceFacade::actorRows() const {
  return buildActorRows(*session_);
}

QVariantList WorkspaceFacade::propertyRows() const {
  return buildPropertyRows(*session_);
}

QVariantList WorkspaceFacade::analysisRows() const {
  return buildAnalysisRows(*session_);
}

QVariantList WorkspaceFacade::annualRows() const {
  return buildAnnualRows(*session_);
}

QVariantList WorkspaceFacade::statementRows() const {
  return buildStatementRows(*session_);
}

QVariantList
WorkspaceFacade::statementTransactionRows(const QString &statementId) const {
  return buildStatementTransactionRows(*session_, statementId);
}

QVariantList WorkspaceFacade::transactionRows() const {
  QVariantList out;
  if (!session_)
    return out;

  const auto &model = session_->models().transactions();
  const int count = model.rowCount();
  out.reserve(count);
  for (int row = 0; row < count; ++row) {
    out.push_back(model.get(row));
  }
  return out;
}

QVariantMap WorkspaceFacade::transaction(const QString &id) const {
  QVariantMap out;
  if (!session_ || id.isEmpty())
    return out;

  const auto &model = session_->models().transactions();
  const int row = model.findRowById(id);
  if (row < 0)
    return out;
  return model.get(row);
}

QVariantMap WorkspaceFacade::annual(const QString &id) const {
  QVariantMap out;
  if (!session_ || id.isEmpty())
    return out;

  const auto &rows = session_->models().annuals().annuals();
  for (const auto &annual : rows) {
    if (!annual)
      continue;
    if (QString::fromStdString(annual->id()) != id)
      continue;

    out = payload::entity::toPayload(*annual);
    out.insert(QStringLiteral("display"),
               annual->name().empty() ? QString::number(annual->year())
                                      : QString::fromStdString(annual->name()));
    return out;
  }

  return out;
}

QVariantMap WorkspaceFacade::annualResultState(const QString &annualId) const {
  if (!coreFacade_ || annualId.isEmpty())
    return {};
  core::application::annual::AnnualService service;
  return ui::annual::toPayload(service.runAnnual(
      coreFacade_->workspaceSnapshot(), ui::annual::toRequest(annualId)));
}

QVariantMap WorkspaceFacade::annualResultStatePreview(
    const QString &annualId, const QVariant &selectedIds, int year) const {
  if (!coreFacade_)
    return {};
  const auto normalizedIds = normalizeAnalysisIds(selectedIds);
  const QString previewId = QStringLiteral("__annual_preview__");
  const auto snapshot = ui::annual::withPreviewAnnual(
      coreFacade_->workspaceSnapshot(), previewId, normalizedIds, year);

  core::application::annual::AnnualService service;
  return ui::annual::toPayload(
      service.runAnnual(snapshot, ui::annual::toRequest(previewId)));
}

QString WorkspaceFacade::analysisIdFromRow(const QVariant &row) const {
  return analysisIdFromVariant(row);
}

QStringList
WorkspaceFacade::normalizeAnalysisIds(const QVariant &values) const {
  QStringList out;
  QSet<QString> seen;
  const QVariantList list = values.toList();
  if (!list.isEmpty()) {
    for (const auto &value : list) {
      const QString id = analysisIdFromVariant(value);
      if (id.isEmpty() || seen.contains(id))
        continue;
      seen.insert(id);
      out.push_back(id);
    }
    return out;
  }

  const QString single = analysisIdFromVariant(values);
  if (!single.isEmpty())
    out.push_back(single);
  return out;
}

QVariantList
WorkspaceFacade::assignedAnnualAnalysisRows(const QVariantList &allRows,
                                            const QVariant &selectedIds) const {
  QVariantList normalizedRows;
  normalizedRows.reserve(allRows.size());
  for (const auto &row : allRows) {
    const QVariantMap normalized = normalizeAnnualAnalysisRow(row);
    if (!normalized.isEmpty())
      normalizedRows.push_back(normalized);
  }

  const QStringList ids = normalizeAnalysisIds(selectedIds);
  QVariantList out;
  out.reserve(ids.size());
  for (const auto &id : ids) {
    for (const auto &rowValue : normalizedRows) {
      const QVariantMap row = rowValue.toMap();
      if (row.value(QStringLiteral("id")).toString() == id) {
        out.push_back(row);
        break;
      }
    }
  }
  return out;
}

QVariantList WorkspaceFacade::availableAnnualAnalysisRows(
    const QVariantList &allRows, const QVariant &selectedIds) const {
  QVariantList normalizedRows;
  normalizedRows.reserve(allRows.size());
  for (const auto &row : allRows) {
    const QVariantMap normalized = normalizeAnnualAnalysisRow(row);
    if (!normalized.isEmpty())
      normalizedRows.push_back(normalized);
  }

  const QStringList selectedIdsList = normalizeAnalysisIds(selectedIds);
  const QSet<QString> selected(selectedIdsList.begin(), selectedIdsList.end());
  QVariantList out;
  for (const auto &rowValue : normalizedRows) {
    const QVariantMap row = rowValue.toMap();
    const QString id = row.value(QStringLiteral("id")).toString();
    if (id.isEmpty() || selected.contains(id))
      continue;
    out.push_back(row);
  }
  return out;
}

QString WorkspaceFacade::addActor(const QString &name,
                                  const QStringList &aliases,
                                  const QStringList &contractIds) {
  if (!coreFacade_)
    return {};
  return QString::fromStdString(
      coreFacade_->addActor(makeActorCommand({}, name, aliases, contractIds)));
}

void WorkspaceFacade::updateActor(const QString &id, const QString &name,
                                  const QStringList &aliases,
                                  const QStringList &contractIds) {
  if (!coreFacade_)
    return;
  coreFacade_->updateActor(makeActorCommand(id, name, aliases, contractIds));
}

QString WorkspaceFacade::saveActor(const QString &id, const QString &name,
                                   const QStringList &aliases,
                                   const QStringList &contractIds) {
  if (!coreFacade_)
    return {};
  if (id.isEmpty())
    return addActor(name, aliases, contractIds);
  updateActor(id, name, aliases, contractIds);
  return id;
}

void WorkspaceFacade::deleteActor(const QString &id) {
  if (coreFacade_)
    coreFacade_->deleteActor(strings::toStdString(id));
}

QString WorkspaceFacade::addProperty(const QString &name,
                                     const QStringList &aliases,
                                     const QStringList &contractIds) {
  if (!coreFacade_)
    return {};
  return QString::fromStdString(coreFacade_->addProperty(
      makePropertyCommand({}, name, aliases, contractIds)));
}

void WorkspaceFacade::updateProperty(const QString &id, const QString &name,
                                     const QStringList &aliases,
                                     const QStringList &contractIds) {
  if (!coreFacade_)
    return;
  coreFacade_->updateProperty(
      makePropertyCommand(id, name, aliases, contractIds));
}

QString WorkspaceFacade::saveProperty(const QString &id, const QString &name,
                                      const QStringList &aliases,
                                      const QStringList &contractIds) {
  if (!coreFacade_)
    return {};
  if (id.isEmpty())
    return addProperty(name, aliases, contractIds);
  updateProperty(id, name, aliases, contractIds);
  return id;
}

void WorkspaceFacade::deleteProperty(const QString &id) {
  if (coreFacade_)
    coreFacade_->deleteProperty(strings::toStdString(id));
}

QString WorkspaceFacade::addContract(const QString &name, const QString &type,
                                     const QStringList &actorIds,
                                     const QStringList &propertyIds,
                                     const QStringList &aliases,
                                     const QString &allocatableMode) {
  if (!coreFacade_)
    return {};
  return QString::fromStdString(coreFacade_->addContract(makeContractCommand(
      {}, name, type, allocatableMode, actorIds, propertyIds, aliases)));
}

void WorkspaceFacade::updateContract(const QString &id, const QString &name,
                                     const QString &type,
                                     const QStringList &actorIds,
                                     const QStringList &propertyIds,
                                     const QStringList &aliases,
                                     const QString &allocatableMode) {
  if (!coreFacade_)
    return;
  coreFacade_->updateContract(makeContractCommand(
      id, name, type, allocatableMode, actorIds, propertyIds, aliases));
}

QString WorkspaceFacade::saveContract(const QString &id, const QString &name,
                                      const QString &type,
                                      const QStringList &actorIds,
                                      const QStringList &propertyIds,
                                      const QStringList &aliases,
                                      const QString &allocatableMode) {
  if (!coreFacade_)
    return {};
  if (id.isEmpty())
    return addContract(name, type, actorIds, propertyIds, aliases,
                       allocatableMode);
  updateContract(id, name, type, actorIds, propertyIds, aliases,
                 allocatableMode);
  return id;
}

void WorkspaceFacade::deleteContract(const QString &id) {
  if (coreFacade_)
    coreFacade_->deleteContract(strings::toStdString(id));
}

QString WorkspaceFacade::addStatement(const QString &name) {
  if (!coreFacade_)
    return {};
  core::ports::workspace::StatementCommand command;
  command.name = strings::toStdString(name);
  return QString::fromStdString(coreFacade_->addStatement(command));
}

void WorkspaceFacade::updateStatement(const QString &id, const QString &name) {
  if (!coreFacade_)
    return;
  core::ports::workspace::StatementCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  coreFacade_->updateStatement(command);
}

QString WorkspaceFacade::saveStatement(const QString &id, const QString &name) {
  if (!coreFacade_)
    return {};
  if (id.isEmpty())
    return addStatement(name);
  updateStatement(id, name);
  return id;
}

void WorkspaceFacade::deleteStatement(const QString &id) {
  if (coreFacade_)
    coreFacade_->deleteStatement(strings::toStdString(id));
}

QString WorkspaceFacade::addTransaction(
    const QString &name, const QString &bookingDate, const QString &valuta,
    double amount, const QString &statementId, int status,
    const QString &actorId, const QString &contractId, bool allocatable,
    const QStringList &propertyIds) {
  if (!coreFacade_)
    return {};
  return QString::fromStdString(coreFacade_->addTransaction(
      makeTransactionCommand({}, name, bookingDate, valuta, amount, statementId,
                             {}, status, actorId, contractId, allocatable,
                             propertyIds)));
}

QString WorkspaceFacade::insertTransactionAfter(
    const QString &afterTransactionId, const QString &name,
    const QString &bookingDate, const QString &valuta, double amount,
    const QString &statementId, int status, const QString &actorId,
    const QString &contractId, bool allocatable,
    const QStringList &propertyIds) {
  if (!coreFacade_)
    return {};
  return QString::fromStdString(coreFacade_->addTransaction(
      makeTransactionCommand({}, name, bookingDate, valuta, amount, statementId,
                             afterTransactionId, status, actorId, contractId,
                             allocatable, propertyIds)));
}

void WorkspaceFacade::updateTransaction(
    const QString &id, const QString &name, const QString &bookingDate,
    const QString &valuta, double amount, const QString &statementId,
    int status, const QString &actorId, const QString &contractId,
    bool allocatable, const QStringList &propertyIds) {
  if (!coreFacade_)
    return;
  coreFacade_->updateTransaction(makeTransactionCommand(
      id, name, bookingDate, valuta, amount, statementId, {}, status, actorId,
      contractId, allocatable, propertyIds));
}

QString WorkspaceFacade::saveTransaction(
    const QString &id, const QString &name, const QString &bookingDate,
    const QString &valuta, double amount, const QString &statementId,
    int status, const QString &actorId, const QString &contractId,
    bool allocatable, const QStringList &propertyIds) {
  if (!coreFacade_)
    return {};
  if (id.isEmpty())
    return addTransaction(name, bookingDate, valuta, amount, statementId,
                          status, actorId, contractId, allocatable,
                          propertyIds);
  updateTransaction(id, name, bookingDate, valuta, amount, statementId, status,
                    actorId, contractId, allocatable, propertyIds);
  return id;
}

void WorkspaceFacade::deleteTransaction(const QString &id) {
  if (coreFacade_)
    coreFacade_->deleteTransaction(strings::toStdString(id));
}

QString WorkspaceFacade::addAnalysis(const QString &name, const QString &type,
                                     const QString &configJson,
                                     const QString &filterSpec,
                                     const QString &exportFormat,
                                     bool includeCalculationAdjustments,
                                     const QString &exportStateJson,
                                     const QString &snapshotTransactionsJson) {
  if (!coreFacade_)
    return {};
  return QString::fromStdString(coreFacade_->addAnalysis(
      makeAnalysisCommand({}, name, type, configJson, filterSpec, exportFormat,
                          includeCalculationAdjustments, exportStateJson,
                          snapshotTransactionsJson)));
}

void WorkspaceFacade::updateAnalysis(
    const QString &id, const QString &name, const QString &type,
    const QString &configJson, const QString &filterSpec,
    const QString &exportFormat, bool includeCalculationAdjustments,
    const QString &exportStateJson, const QString &snapshotTransactionsJson) {
  if (!coreFacade_)
    return;
  std::vector<std::pair<std::string, double>> adjustments;
  const std::string targetId = strings::toStdString(id);
  for (const auto &analysis : coreFacade_->workspaceSnapshot().analyses) {
    if (analysis.id == targetId) {
      adjustments = analysis.adjustments;
      break;
    }
  }
  coreFacade_->updateAnalysis(
      makeAnalysisCommand(id, name, type, configJson, filterSpec, exportFormat,
                          includeCalculationAdjustments, exportStateJson,
                          snapshotTransactionsJson, std::move(adjustments)));
}

void WorkspaceFacade::updateAnalysis(
    const QString &id, const QString &name, const QString &type,
    const QString &configJson, const QString &filterSpec,
    const QString &exportFormat, bool includeCalculationAdjustments,
    const QString &exportStateJson, const QString &snapshotTransactionsJson,
    const QString &adjustmentsJson) {
  if (!coreFacade_)
    return;
  coreFacade_->updateAnalysis(
      makeAnalysisCommand(id, name, type, configJson, filterSpec, exportFormat,
                          includeCalculationAdjustments, exportStateJson,
                          snapshotTransactionsJson,
                          analysisAdjustmentsFromJson(adjustmentsJson)));
}

void WorkspaceFacade::deleteAnalysis(const QString &id) {
  if (coreFacade_)
    coreFacade_->deleteAnalysis(strings::toStdString(id));
}

QString WorkspaceFacade::addAnnual(const QString &name, int year,
                                   const QStringList &analysisIds) {
  if (!coreFacade_)
    return {};
  return QString::fromStdString(
      coreFacade_->addAnnual(makeAnnualCommand({}, name, year, analysisIds)));
}

void WorkspaceFacade::updateAnnual(const QString &id, const QString &name,
                                   int year, const QStringList &analysisIds) {
  if (!coreFacade_)
    return;
  coreFacade_->updateAnnual(makeAnnualCommand(id, name, year, analysisIds));
}

QString WorkspaceFacade::saveAnnual(const QString &id, const QString &name,
                                    int year, const QStringList &analysisIds) {
  if (!coreFacade_)
    return {};
  if (id.isEmpty())
    return addAnnual(name, year, analysisIds);
  updateAnnual(id, name, year, analysisIds);
  return id;
}

void WorkspaceFacade::deleteAnnual(const QString &id) {
  if (coreFacade_)
    coreFacade_->deleteAnnual(strings::toStdString(id));
}

TransactionFilter *
WorkspaceFacade::statementTransactions(const QString &statementId) {
  return session_->statementTransactions(statementId);
}

TransactionFilter *
WorkspaceFacade::propertyTransactions(const QString &propertyId) {
  return session_->propertyTransactions(propertyId);
}

void WorkspaceFacade::applyDeletionImpact(const DeletionImpact &impact) {
  session_->applyDeletionImpact(impact);
  bumpDataRevision();
}

void WorkspaceFacade::setTransactionPropertyIdsImmediate(
    const QString &txId, const QStringList &propertyIds) {
  session_->setTransactionPropertyIdsImmediate(txId, propertyIds);
  bumpDataRevision();
}

} // namespace ui
