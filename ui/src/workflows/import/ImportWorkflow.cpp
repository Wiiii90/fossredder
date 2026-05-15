/**
 * @file ui/src/workflows/import/ImportWorkflow.cpp
 * @brief Implements the import workflow used by the QML UI.
 */

#include "ui/workflows/import/ImportWorkflow.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QRegularExpression>
#include <QUuid>

#include <algorithm>
#include <exception>
#include <map>
#include <stdexcept>
#include <string>

#include "core/application/import/ImportLog.h"
#include "core/application/import/transaction/AmountParser.h"
#include "core/constants/import.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/values/Alias.h"
#include "core/jobs/ImportJobSpec.h"
#include "core/jobs/JobSystem.h"
#include "core/jobs/JobTypes.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "core/ports/workspace/IWorkspaceReader.h"
#include "ui/shared/config/Defaults.h"
#include "ui/adapters/core/DraftViewMapper.h"
#include "ui/adapters/core/ImportDraftMapper.h"
#include "ui/workflows/import/ImportRunStore.h"
#include "ui/shared/observability/Origins.h"
#include "ui/shared/observability/Trace.h"
#include "ui/shared/payload/PayloadMapper.h"
#include "ui/shared/text/Text.h"
#include "ui/shared/util/CoreFacadeGuard.h"
#include "ui/shared/util/StringConversions.h"

namespace ui {

namespace {

/** @brief Returns an ISO timestamp for import run bookkeeping. */
QString currentTimestamp() {
  return QDateTime::currentDateTime().toString(
      QStringLiteral("dd.MM.yyyy HH:mm:ss"));
}

QString generateLogId() {
  return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

ImportRunRow
toRunRow(const std::shared_ptr<core::application::importing::ImportLog> &log) {
  ImportRunRow row;
  if (!log)
    return row;
  row.logId = QString::fromStdString(log->id);
  row.time = QString::fromStdString(log->time);
  row.type = QString::fromStdString(log->type);
  row.file = QString::fromStdString(log->file);
  row.status = QString::fromStdString(log->status);
  row.message = QString::fromStdString(log->message);
  row.draftAttached = log->draftAttached;
  row.draftId = QString::fromStdString(log->draftId);
  row.statementId = QString::fromStdString(log->statementId);
  return row;
}

core::application::importing::ImportLog toImportLog(const ImportRunRow &row) {
  core::application::importing::ImportLog log;
  log.id = strings::toStdString(row.logId);
  log.time = strings::toStdString(row.time);
  log.type = strings::toStdString(row.type);
  log.file = strings::toStdString(row.file);
  log.status = strings::toStdString(row.status);
  log.message = strings::toStdString(row.message);
  log.draftAttached = row.draftAttached;
  log.draftId = strings::toStdString(row.draftId);
  log.statementId = strings::toStdString(row.statementId);
  return log;
}

/** @brief Clamps import progress values to the configured UI range. */
double clampProgress(double progress) {
  if (progress < ui::config::importProgress::kMinimum)
    return ui::config::importProgress::kMinimum;
  if (progress > ui::config::importProgress::kMaximum)
    return ui::config::importProgress::kMaximum;
  return progress;
}

/** @brief Builds the import job specification and prepares a fresh import run
 * folder. */
core::jobs::ImportStatementJobSpec buildImportSpec(const QString &path,
                                                   QString &runRoot) {
  importing::cleanupOldImportRuns(core::constants::importing::runs::kKeepCount);

  const auto runInfo = importing::createImportRunInfo();
  runRoot = runInfo.runRoot;

  core::jobs::ImportStatementJobSpec spec;
  spec.sourcePath = strings::toEncodedPath(path);
  spec.runRoot = strings::toEncodedPath(runInfo.runRoot);

  const QByteArray runIdNative = runInfo.runIdPrefix.toUtf8();
  spec.runIdPrefix = {runIdNative.constData(),
                      static_cast<size_t>(runIdNative.size())};
  return spec;
}

core::domain::Alias makeAlias(const QString& value)
{
  const std::string stdValue = ui::strings::toStdString(value);
  return core::domain::Alias{stdValue, {}, stdValue, {}, {}};
}

const ui::TransactionDraft* currentDraft(ui::StatementDraft* draft)
{
  if (!draft || !draft->hasCurrent()) {
    return nullptr;
  }

  const auto index = draft->currentIndex();
  const auto& drafts = draft->transactions()->drafts();
  if (index < 0 || static_cast<std::size_t>(index) >= drafts.size()) {
    return nullptr;
  }
  return &drafts[static_cast<std::size_t>(index)];
}

core::domain::catalog::WorkspaceCatalog toWorkspaceState(
    const core::ports::workspace::WorkspaceSnapshot& snapshot)
{
  core::domain::catalog::WorkspaceCatalog state;

  core::domain::catalog::WorkspaceCatalog::ActorList actors;
  actors.reserve(snapshot.actors.size());
  for (const auto& src : snapshot.actors) {
    auto entity = std::make_shared<core::domain::Actor>();
    entity->setId(src.id);
    entity->rename(src.name);
    std::vector<core::domain::Alias> aliases;
    aliases.reserve(src.aliases.size());
    for (const auto& alias : src.aliases) {
      aliases.emplace_back(alias.value, alias.kind, alias.source, alias.createdAt, alias.updatedAt);
    }
    entity->setAliases(std::move(aliases));
    entity->setCreatedAt(src.createdAt);
    entity->setUpdatedAt(src.updatedAt);
    actors.push_back(std::move(entity));
  }
  state.setActors(std::move(actors));

  core::domain::catalog::WorkspaceCatalog::PropertyList properties;
  properties.reserve(snapshot.properties.size());
  for (const auto& src : snapshot.properties) {
    auto entity = std::make_shared<core::domain::Property>();
    entity->setId(src.id);
    entity->rename(src.name);
    std::vector<core::domain::Alias> aliases;
    aliases.reserve(src.aliases.size());
    for (const auto& alias : src.aliases) {
      aliases.emplace_back(alias.value, alias.kind, alias.source, alias.createdAt, alias.updatedAt);
    }
    entity->setAliases(std::move(aliases));
    entity->setCreatedAt(src.createdAt);
    entity->setUpdatedAt(src.updatedAt);
    properties.push_back(std::move(entity));
  }
  state.setProperties(std::move(properties));

  core::domain::catalog::WorkspaceCatalog::ContractList contracts;
  contracts.reserve(snapshot.contracts.size());
  for (const auto& src : snapshot.contracts) {
    auto entity = std::make_shared<core::domain::Contract>();
    entity->setId(src.id);
    entity->rename(src.name);
    entity->setType(src.type);
    entity->setActorIds(src.actorIds);
    entity->setPropertyIds(src.propertyIds);
    std::vector<core::domain::Alias> aliases;
    aliases.reserve(src.aliases.size());
    for (const auto& alias : src.aliases) {
      aliases.emplace_back(alias.value, alias.kind, alias.source, alias.createdAt, alias.updatedAt);
    }
    entity->setAliases(std::move(aliases));
    entity->setCreatedAt(src.createdAt);
    entity->setUpdatedAt(src.updatedAt);
    contracts.push_back(std::move(entity));
  }
  state.setContracts(std::move(contracts));

  core::domain::catalog::WorkspaceCatalog::StatementList statements;
  statements.reserve(snapshot.statements.size());
  for (const auto& src : snapshot.statements) {
    auto entity = std::make_shared<core::domain::Statement>();
    entity->setId(src.id);
    entity->rename(src.name);
    entity->setTransactionIds(src.transactionIds);
    entity->setCreatedAt(src.createdAt);
    entity->setUpdatedAt(src.updatedAt);
    statements.push_back(std::move(entity));
  }
  state.setStatements(std::move(statements));

  core::domain::catalog::WorkspaceCatalog::TransactionList transactions;
  transactions.reserve(snapshot.transactions.size());
  for (const auto& src : snapshot.transactions) {
    auto entity = std::make_shared<core::domain::Transaction>();
    entity->setId(src.id);
    entity->setName(src.name);
    entity->setBookingDate(src.bookingDate);
    entity->setValuta(src.valuta);
    entity->setAmount(src.amount);
    entity->setStatus(src.status);
    entity->setContractId(src.contractId);
    entity->setActorId(src.actorId);
    entity->setStatementId(src.statementId);
    entity->setAllocatable(src.allocatable);
    entity->setPropertyIds(src.propertyIds);
    entity->setCreatedAt(src.createdAt);
    entity->setUpdatedAt(src.updatedAt);
    transactions.push_back(std::move(entity));
  }
  state.setTransactions(std::move(transactions));

  return state;
}

core::ports::workspace::StatementDraftSnapshot toSnapshot(
    const core::application::importing::draft::StatementDraft& draft)
{
  core::ports::workspace::StatementDraftSnapshot snapshot;
  snapshot.id = draft.id;
  snapshot.name = draft.name;
  snapshot.transactionIds = draft.transactionIds;
  snapshot.createdAt = draft.createdAt;
  snapshot.updatedAt = draft.updatedAt;
  snapshot.transactions.reserve(draft.transactions.size());
  for (const auto& tx : draft.transactions) {
    core::ports::workspace::TransactionDraftSnapshot txSnapshot;
    txSnapshot.id = tx.id;
    txSnapshot.statementDraftId = tx.statementDraftId;
    txSnapshot.name = tx.name;
    txSnapshot.bookingDate = tx.bookingDate;
    txSnapshot.valuta = tx.valuta;
    txSnapshot.amount = tx.amount;
    txSnapshot.actorId = tx.actorId;
    txSnapshot.contractId = tx.contractId;
    txSnapshot.propertyIds = tx.propertyIds;
    txSnapshot.status = static_cast<int>(tx.status);
    txSnapshot.allocatable = tx.allocatable;
    txSnapshot.position = tx.position;
    txSnapshot.metadata = tx.metadata;
    snapshot.transactions.push_back(std::move(txSnapshot));
  }
  return snapshot;
}

} // namespace

} // namespace

namespace ui {

ImportWorkflow::ImportWorkflow(
    JobSystemFactory jobSystemFactory,
    std::shared_ptr<core::errors::IErrorReporter> errorReporter,
    std::shared_ptr<core::ports::presenters::IImportPresenter> importPresenter,
    std::shared_ptr<core::application::importing::draft::IImportMatcherService>
        importMatcherService,
    core::ports::workspace::IWorkspaceWriter *workspaceWriter,
    QObject *parent)
    : QObject(parent), runs_(std::make_unique<ImportRunList>(this)),
      jobSystemFactory_(std::move(jobSystemFactory)),
      errorReporter_(std::move(errorReporter)),
      importPresenter_(std::move(importPresenter)),
      importMatcherService_(std::move(importMatcherService)),
      workspaceWriter_(workspaceWriter) {
  if (!errorReporter_)
    throw std::invalid_argument("ImportWorkflow requires an error reporter");
  state_.setMatcherService(importMatcherService_);
}

void ImportWorkflow::setStateSnapshotProvider(
    StateSnapshotProvider provider) {
  stateSnapshotProvider_ = std::move(provider);
  if (stateSnapshotProvider_) {
    const auto snapshot = stateSnapshotProvider_();
    restoreRunsFromSnapshot(snapshot);
    emit stateChanged();
  }
}

void ImportWorkflow::setStatementDraftStore(StatementDraftStore store) {
  statementDraftStore_ = std::move(store);
}

void ImportWorkflow::activateRunAt(int index) {
  const auto row = runs_->at(index);
  if (row.logId.isEmpty() || !row.draftAttached)
    return;
  activeDraftLogId_ = !row.draftId.isEmpty() ? row.draftId : row.logId;
}

bool ImportWorkflow::openPrevDraft() {
  const auto index = activeDraftStackIndex();
  const auto ids = draftStackIds();
  if (index <= 0 || index >= ids.size())
    return false;
  return openPersistedDraft(ids.at(index - 1));
}

bool ImportWorkflow::openNextDraft() {
  const auto index = activeDraftStackIndex();
  const auto ids = draftStackIds();
  if (index < 0 || index >= ids.size() - 1)
    return false;
  return openPersistedDraft(ids.at(index + 1));
}

core::domain::catalog::WorkspaceCatalog ImportWorkflow::matchingStateForDraft(
    const ui::StatementDraft *draft) const {
  const auto reader = workspaceWriter_
                          ? dynamic_cast<const core::ports::workspace::IWorkspaceReader *>(workspaceWriter_)
                          : nullptr;
  const auto liveState = reader ? toWorkspaceState(reader->workspaceSnapshot())
                                : core::domain::catalog::WorkspaceCatalog{};
  if (!draft || !importMatcherService_) {
    return liveState;
  }

  return importMatcherService_->withFallbackState(
      draft->hasCatalogState() ? draft->catalogState()
                               : core::domain::catalog::WorkspaceCatalog{},
      liveState);
}

core::application::importing::draft::StatementDraft
ImportWorkflow::buildFinalizationInput(ui::StatementDraft *draft) const {
  core::application::importing::draft::StatementDraft input;
  if (!draft || !this->workspaceWriter_ || !this->importMatcherService_) {
    return input;
  }

  const auto* draftTransactions = draft->transactions();
  std::vector<ui::TransactionDraft> transactionDrafts;
  if (draftTransactions) {
    transactionDrafts = draftTransactions->drafts();
  }
  input.id = ui::strings::toStdString(draft->draftId());
  input.name = ui::strings::toStdString(draft->name());
  input.transactions.reserve(transactionDrafts.size());

  const auto appState = matchingStateForDraft(draft);
  const auto* matcher = this->importMatcherService_.get();
  for (const ui::TransactionDraft &draftTransaction : transactionDrafts) {
    core::application::importing::draft::TransactionDraft transaction;
    transaction.name = ui::strings::toStdString(draftTransaction.name);
    transaction.bookingDate = ui::strings::toStdString(draftTransaction.bookingDate);
    transaction.valuta = ui::strings::toStdString(draftTransaction.valuta);
    transaction.amount = draftTransaction.amount;
    if (!draftTransaction.proofImageData.isEmpty()) {
      const QByteArray imageBytes =
          QByteArray::fromBase64(draftTransaction.proofImageData.toLatin1());
      if (!imageBytes.isEmpty()) {
        transaction.proofImageData.assign(imageBytes.begin(), imageBytes.end());
      }
    }
    transaction.status = static_cast<core::domain::Transaction::Status>(draftTransaction.status);

    QString actorId = draftTransaction.actorId;
    if (actorId.isEmpty() &&
        (!draftTransaction.actorText.trimmed().isEmpty() || draftTransaction.actorSelected)) {
      actorId = QString::fromStdString(matcher->resolveActorId(
          appState, ui::strings::toStdString(draftTransaction.actorText)));
      if (actorId.isEmpty()) {
        const QString actorName = draftTransaction.actorText.trimmed();
        if (!actorName.isEmpty()) {
          core::ports::workspace::ActorCommand command;
          command.name = ui::strings::toStdString(actorName);
          const auto alias = makeAlias(actorName);
          command.aliases.push_back({alias.value(), alias.kind(), alias.source(),
                                     alias.hitCount(), alias.lastUsedAt(),
                                     alias.createdAt(), alias.updatedAt()});
          actorId = QString::fromStdString(
              this->workspaceWriter_->addActor(command));
        }
      }
    }

    QString contractId = draftTransaction.contractId;
    if (contractId.isEmpty() && draftTransaction.contractSelected) {
      const QString contractType = draftTransaction.type.trimmed();
      if (!contractType.isEmpty()) {
        std::vector<std::string> actorIds;
        if (!actorId.isEmpty()) {
          actorIds.push_back(ui::strings::toStdString(actorId));
        }
        std::vector<std::string> propertyIds =
            ui::strings::toStdList(draftTransaction.propertyIds);
        std::vector<core::domain::Alias> aliases;
        for (const auto &value :
             matcher->referenceAliasesFromMetadata(
                 ui::strings::toStdString(draftTransaction.metadata))) {
          aliases.push_back(core::domain::Alias{value, {}, value, {}, {}});
        }
        core::ports::workspace::ContractCommand command;
        command.name = {};
        command.type = ui::strings::toStdString(contractType);
        command.actorIds = std::move(actorIds);
        command.propertyIds = std::move(propertyIds);
        for (const auto &alias : aliases) {
          command.aliases.push_back({alias.value(), alias.kind(),
                                     alias.source(), alias.hitCount(),
                                     alias.lastUsedAt(), alias.createdAt(),
                                     alias.updatedAt()});
        }
        contractId = QString::fromStdString(
            this->workspaceWriter_->addContract(command));
      }
    }

    transaction.actorId = ui::strings::toStdString(actorId);
    transaction.contractId = ui::strings::toStdString(contractId);
    transaction.allocatable =
        draftTransaction.allocatableSelected
            ? draftTransaction.allocatable
             : (matcher->contractIsFullyAllocatable(
                    appState, ui::strings::toStdString(contractId)) ||
                draftTransaction.allocatable);
    transaction.propertyIds = ui::strings::toStdList(draftTransaction.propertyIds);
    transaction.type = ui::strings::toStdString(draftTransaction.type);
    input.transactions.push_back(std::move(transaction));
  }

  return input;
}

void ImportWorkflow::syncCurrentTransactionDraftImpl(ui::StatementDraft *draft) {
  const auto *current = currentDraft(draft);
  if (!draft || !current || !workspaceWriter_ || !importMatcherService_) {
    return;
  }

  const auto appState = matchingStateForDraft(draft);
  const auto derived = importMatcherService_->buildDraftDerivedState(
      appState, ui::importing::toCoreSelection(*current));
  const auto index = draft->currentIndex();
  bool changed = false;

  if (current->actorId.isEmpty()) {
    if (derived.actorCurrentIndex > 0 &&
        static_cast<std::size_t>(derived.actorCurrentIndex) <
            derived.actorChoices.size()) {
      const auto &actorRow =
          derived.actorChoices[static_cast<std::size_t>(derived.actorCurrentIndex)];
      if (!actorRow.synthetic && !actorRow.id.empty()) {
        const QString actorId = QString::fromStdString(actorRow.id);
        const QString actorText = ui::importing::choiceDisplayText(actorRow);
        changed = changed || current->actorId != actorId || current->actorText != actorText || current->actorSelected;
        draft->transactions()->setActorId(index, actorId);
        draft->transactions()->setActorText(index, actorText);
        draft->transactions()->setActorSelected(index, false);
      }
    } else if (!derived.actorSeedText.empty()) {
      const QString actorText = QString::fromStdString(derived.actorSeedText);
      changed = changed || current->actorText != actorText;
      draft->transactions()->setActorText(index, actorText);
    }
  }

  if (current->contractId.isEmpty()) {
    if (derived.contractCurrentIndex > 0 &&
        static_cast<std::size_t>(derived.contractCurrentIndex) <
            derived.contractChoices.size()) {
      const auto &contractRow =
          derived.contractChoices[static_cast<std::size_t>(derived.contractCurrentIndex)];
      if (!contractRow.synthetic && !contractRow.id.empty()) {
        const QString contractId = QString::fromStdString(contractRow.id);
        changed = changed || current->contractId != contractId || current->contractSelected;
        draft->transactions()->setContractId(index, contractId);
        if (!contractRow.type.empty()) {
          const QString type = QString::fromStdString(contractRow.type);
          changed = changed || current->type != type;
          draft->transactions()->setType(index, type);
        }
        if (!contractRow.actorIds.empty()) {
          const QString actorId = QString::fromStdString(contractRow.actorIds.front());
          changed = changed || current->actorId != actorId || current->actorSelected;
          draft->transactions()->setActorId(index, actorId);
          if (const auto *actorRow = ui::importing::findChoiceRowById(
                  derived.actorChoices, contractRow.actorIds.front())) {
            const QString actorText = ui::importing::choiceDisplayText(*actorRow);
            changed = changed || current->actorText != actorText;
            draft->transactions()->setActorText(index, actorText);
          }
          draft->transactions()->setActorSelected(index, false);
        }
        if (!contractRow.propertyIds.empty()) {
          const auto propertyIds =
              ui::payload::mapper::toQStringList(contractRow.propertyIds);
          changed = changed || current->propertyIds != propertyIds;
          draft->transactions()->setProperties(index, propertyIds);
        }
        draft->transactions()->setContractSelected(index, true);
      }
    } else if (!derived.contractSeedText.empty()) {
      const QString type = QString::fromStdString(derived.contractSeedText);
      changed = changed || current->type != type;
      draft->transactions()->setType(index, type);
    } else if (!current->contractSelected) {
      changed = changed || !current->type.isEmpty();
      draft->transactions()->setType(index, QString());
    }
  }

  if (current->propertyIds.isEmpty() && !derived.autoPropertyIds.empty()) {
    const auto propertyIds = ui::payload::mapper::toQStringList(derived.autoPropertyIds);
    changed = changed || current->propertyIds != propertyIds;
    draft->transactions()->setProperties(index, propertyIds);
  }

  if (!current->allocatableSelected) {
    changed = changed || current->allocatable != derived.effectiveAllocatable;
    draft->transactions()->setAllocatable(index, derived.effectiveAllocatable);
  }

  if (changed) {
    draft->refresh();
  }
}

QString ImportWorkflow::finalizeStatementDraft(ui::StatementDraft *draft) {
  if (!draft || !workspaceWriter_) return {};

  return ui::util::guard::invokeValue<QString>(
      workspaceWriter_, ui::observability::origins::workflow::import::kFinalize, {},
      [&]() {
        const auto input = buildFinalizationInput(draft);
        if (input.transactions.empty()) return QString{};
        core::ports::workspace::FinalizeStatementDraftCommand command;
        command.draft = toSnapshot(input);
        return QString::fromStdString(
            workspaceWriter_->finalizeStatementDraft(command));
      });
}

void ImportWorkflow::persistStatementDraft(ui::StatementDraft *draft) {
  ui::util::guard::invokeVoid(
      workspaceWriter_, ui::observability::origins::workflow::import::kFinalize,
      [&]() {
        if (!draft || !workspaceWriter_) return;
        auto input = buildFinalizationInput(draft);
        if (input.transactions.empty()) return;
        core::ports::workspace::StatementDraftCommand command;
        command.draft = toSnapshot(input);
        workspaceWriter_->saveStatementDraft(command);
      });
}

void ImportWorkflow::clearPersistedStatementDraft(const QString &draftId) {
  ui::util::guard::invokeVoid(workspaceWriter_,
                              ui::observability::origins::workflow::import::kFinalize,
                              [&]() {
                                if (workspaceWriter_)
                                  workspaceWriter_->clearStatementDraft(
                                      strings::toStdString(draftId));
                              });
}

QVariantMap ImportWorkflow::currentTransactionViewState(ui::StatementDraft *draft) const {
  return ui::util::guard::invokeValue<QVariantMap>(
      workspaceWriter_, ui::observability::origins::workflow::import::kFinalize, {}, [&]() {
        const auto *current = currentDraft(draft);
        if (!current || !importMatcherService_) return QVariantMap{};
        return ui::importing::toViewState(importMatcherService_->buildDraftDerivedState(
            matchingStateForDraft(draft),
            ui::importing::toCoreSelection(*current)));
      });
}

QVariantMap ImportWorkflow::findChoiceRowByText(const QVariantList &rows,
                                                const QString &text) const {
  for (const auto &item : rows) {
    const QVariantMap row = item.toMap();
    if (!row.isEmpty() && ui::importing::rowMatchesText(row, text))
      return row;
  }
  return {};
}

void ImportWorkflow::syncCurrentTransactionDraft(ui::StatementDraft *draft) {
  syncCurrentTransactionDraftImpl(draft);
}

void ImportWorkflow::selectCurrentActorChoice(ui::StatementDraft *draft,
                                              const QVariantMap &row) {
  if (!draft || row.isEmpty()) return;
  const auto currentIndex = draft->currentIndex();
  if (currentIndex < 0) return;
  draft->transactions()->setActorId(currentIndex, row.value(QStringLiteral("id")).toString());
  draft->transactions()->setActorText(currentIndex, row.value(QStringLiteral("display")).toString());
  draft->transactions()->setActorSelected(currentIndex, true);
  draft->refresh();
}

void ImportWorkflow::selectCurrentContractChoice(ui::StatementDraft *draft,
                                                const QVariantMap &row) {
  if (!draft || row.isEmpty()) return;
  const auto currentIndex = draft->currentIndex();
  if (currentIndex < 0) return;
  draft->transactions()->setContractId(currentIndex, row.value(QStringLiteral("id")).toString());
  draft->transactions()->setContractSelected(currentIndex, true);
  draft->refresh();
}

void ImportWorkflow::setCurrentPropertySelected(ui::StatementDraft *draft,
                                                const QString &propertyId,
                                                bool selected) {
  const auto *current = currentDraft(draft);
  if (!draft || !current) return;
  const auto currentIndex = draft->currentIndex();
  if (currentIndex < 0) return;
  auto propertyIds = current->propertyIds;
  if (selected) {
    if (!propertyIds.contains(propertyId))
      propertyIds.push_back(propertyId);
  } else {
    propertyIds.removeAll(propertyId);
  }
  draft->transactions()->setProperties(currentIndex, propertyIds);
  draft->refresh();
}

void ImportWorkflow::updateCurrentAmount(ui::StatementDraft *draft,
                                        const QString &text) {
  if (!draft) return;
  const auto currentIndex = draft->currentIndex();
  if (currentIndex < 0) return;
  const auto parsed = core::application::importing::transaction::parseAmountString(text.toStdString());
  if (!parsed) return;
  draft->transactions()->setAmount(currentIndex, *parsed);
  draft->refresh();
}

void ImportWorkflow::setImportLogsStore(ImportLogsStore store) {
  importLogsStore_ = std::move(store);
}

QString ImportWorkflow::selectedFile() const { return state_.selectedFile(); }

QStringList ImportWorkflow::queuedFiles() const {
  return state_.queuedFiles();
}

ui::StatementDraft *ImportWorkflow::draft() const noexcept {
  return state_.draft();
}

bool ImportWorkflow::hasPrevDraft() const {
  const auto index = activeDraftStackIndex();
  return index > 0;
}

bool ImportWorkflow::hasNextDraft() const {
  const auto index = activeDraftStackIndex();
  const auto ids = draftStackIds();
  return index >= 0 && index < ids.size() - 1;
}

ImportRunList *ImportWorkflow::runs() noexcept { return runs_.get(); }

void ImportWorkflow::addRunNote(const QString &status, const QString &message,
                                  bool draftAttached,
                                  const QString &statementId) {
  if (draftAttached || !activeDraftLogId_.isEmpty() || !statementId.isEmpty()) {
    upsertActiveDraftRun(status, message, draftAttached, statementId);
  } else {
    runs_->addRun(currentTimestamp(), ui::text::importRuns::typeStatement(),
                  state_.currentRunFile(), status, message, false, statementId,
                  generateLogId());
  }
  persistRuns();
  emit stateChanged();
}

void ImportWorkflow::removeRunAt(int index) {
  const auto row = runs_->at(index);
  if (row.logId == activeDraftLogId_) {
    activeDraftLogId_.clear();
  }
  runs_->removeAt(index);
  persistRuns();
  emit stateChanged();
}

void ImportWorkflow::restoreRunsFromSnapshot(
    const core::application::workspace::WorkspaceSessionState &snapshot) {
  std::vector<ImportRunRow> rows;
  rows.reserve(snapshot.workflow.importLogs.size());
  activeDraftLogId_.clear();

  for (const auto &item : snapshot.workflow.importLogs) {
    if (!item)
      continue;
    auto row = toRunRow(item);
    if (row.logId.isEmpty())
      row.logId = generateLogId();
    if (row.draftAttached && row.draftId.isEmpty())
      row.draftId = row.logId;
    if (row.draftAttached && activeDraftLogId_.isEmpty())
      activeDraftLogId_ = row.logId;
    rows.push_back(std::move(row));
  }
  runs_->setRuns(std::move(rows));
}

void ImportWorkflow::persistRuns() {
  if (!importLogsStore_)
    return;
  auto items = runs_->snapshot();
  bool normalized = false;
  for (auto &row : items) {
    if (!row.logId.isEmpty())
      continue;
    row.logId = generateLogId();
    runs_->upsertRun(row);
    normalized = true;
  }
  if (normalized) {
    items = runs_->snapshot();
  }
  std::vector<core::application::importing::ImportLog> logs;
  logs.reserve(items.size());
  for (const auto &row : items) {
    logs.push_back(toImportLog(row));
  }
  importLogsStore_(logs);
}

QString ImportWorkflow::resolveDraftContextLogId() const {
  if (!activeDraftLogId_.isEmpty())
    return activeDraftLogId_;
  if (state_.draft() && !state_.draft()->draftId().isEmpty())
    return state_.draft()->draftId();
  return {};
}

QStringList ImportWorkflow::draftStackIds() const {
  QStringList ids;
  const auto rows = runs_->snapshot();
  for (const auto &row : rows) {
    if (!row.draftAttached)
      continue;
    const QString id = !row.draftId.isEmpty() ? row.draftId : row.logId;
    if (id.isEmpty() || ids.contains(id))
      continue;
    ids.push_back(id);
  }
  return ids;
}

int ImportWorkflow::activeDraftStackIndex() const {
  const auto ids = draftStackIds();
  if (ids.isEmpty())
    return -1;

  QString activeId = resolveDraftContextLogId();
  if (activeId.isEmpty() && state_.draft() &&
      !state_.draft()->draftId().isEmpty()) {
    activeId = state_.draft()->draftId();
  }

  if (activeId.isEmpty())
    return -1;
  return ids.indexOf(activeId);
}

ImportRunRow ImportWorkflow::upsertRunById(
    const QString &logId, const QString &status, const QString &message,
    bool draftAttached, const QString &draftId, const QString &statementId) {
  ImportRunRow row;
  row.logId = logId.isEmpty() ? generateLogId() : logId;
  const int existingIndex = runs_->findByLogId(row.logId);
  if (existingIndex >= 0) {
    row = runs_->at(existingIndex);
  } else {
    row.type = ui::text::importRuns::typeStatement();
    row.file = state_.currentRunFile();
  }
  row.time = currentTimestamp();
  row.status = status;
  row.message = message;
  row.draftAttached = draftAttached;
  row.draftId = draftId;
  row.statementId = statementId;
  runs_->upsertRun(row);
  return row;
}

ImportRunRow ImportWorkflow::upsertActiveDraftRun(
    const QString &status, const QString &message, bool draftAttached,
    const QString &statementId) {
  const QString logId = resolveDraftContextLogId();
  const QString draftId = draftAttached ? logId : QString();
  auto row = upsertRunById(logId, status, message, draftAttached, draftId,
                           statementId);

  if (draftAttached) {
    activeDraftLogId_ = row.logId;
  } else if (!activeDraftLogId_.isEmpty() && row.logId == activeDraftLogId_) {
    activeDraftLogId_.clear();
  }
  return row;
}

bool ImportWorkflow::ensureJobBridge() {
  if (jobBridge_)
    return true;
  if (!jobSystemFactory_)
    return false;

  auto jobSystem = jobSystemFactory_();
  if (!jobSystem)
    return false;

  jobBridge_ =
      std::make_unique<importing::ImportJobBridge>(std::move(jobSystem));
  jobBridge_->setExceptionReporter(
      [this](const char *origin, std::exception_ptr exception) {
        reportException(origin, exception);
      });
  return true;
}

void ImportWorkflow::addFiles(const QStringList &paths) {
  if (state_.addFiles(paths))
    emit stateChanged();
}

void ImportWorkflow::setSelectedFile(const QString &path) {
  if (state_.setSelectedFile(path))
    emit stateChanged();
}

void ImportWorkflow::resetStatus() {
  if (state_.resetStatus())
    emit stateChanged();
}

void ImportWorkflow::clearDraft() {
  const bool shouldStartNext = state_.clearDraft();
  emit stateChanged();
  if (shouldStartNext)
    startNextQueuedImport();
}

bool ImportWorkflow::hasPersistedDraft() const {
  if (!stateSnapshotProvider_)
    return false;
  const auto snapshot = stateSnapshotProvider_();
  return !snapshot.workflow.statementDrafts.empty() &&
         !snapshot.workflow.transactionDrafts.empty();
}

bool ImportWorkflow::openPersistedDraft(const QString &logId) {
  if (!stateSnapshotProvider_)
    return false;

  QString requestedLogId = !logId.isEmpty() ? logId : activeDraftLogId_;
  if (requestedLogId.isEmpty()) {
    const auto rows = runs_->snapshot();
    for (const auto &row : rows) {
      if (!row.draftAttached)
        continue;
      requestedLogId = !row.draftId.isEmpty() ? row.draftId : row.logId;
      if (!requestedLogId.isEmpty())
        break;
    }
  }
  if (!requestedLogId.isEmpty()) {
    activeDraftLogId_ = requestedLogId;
  }

  const auto snapshot = stateSnapshotProvider_();
  const bool restored = restoreDraftFromState(snapshot);
  if (restored) {
    emit stateChanged();
  }
  return restored;
}

bool ImportWorkflow::restoreDraftFromState(
    const core::application::workspace::WorkspaceSessionState &snapshot) {
  if (snapshot.workflow.statementDrafts.empty()) {
    return false;
  }

  std::shared_ptr<core::application::importing::draft::StatementDraft>
      persistedStatementDraft;
  const QString requestedDraftId = resolveDraftContextLogId();
  if (!requestedDraftId.isEmpty()) {
    for (const auto &draft : snapshot.workflow.statementDrafts) {
      if (draft && QString::fromStdString(draft->id) == requestedDraftId) {
        persistedStatementDraft = draft;
        break;
      }
    }
  }
  if (!persistedStatementDraft) {
    persistedStatementDraft = snapshot.workflow.statementDrafts.front();
  }
  if (!persistedStatementDraft) {
    return false;
  }

  const QString restoredDraftId =
      QString::fromStdString(persistedStatementDraft->id);
  if (!restoredDraftId.isEmpty()) {
    activeDraftLogId_ = restoredDraftId;
  }

  std::vector<core::application::importing::draft::TransactionDraft> txDrafts;
  txDrafts.reserve(snapshot.workflow.transactionDrafts.size());
  for (const auto &tx : snapshot.workflow.transactionDrafts) {
    if (!tx)
      continue;
    if (!persistedStatementDraft->id.empty() &&
        tx->statementDraftId != persistedStatementDraft->id)
      continue;
    txDrafts.push_back(*tx);
  }

  std::sort(txDrafts.begin(), txDrafts.end(),
            [](const auto &lhs, const auto &rhs) {
              return lhs.position < rhs.position;
            });

  auto statement = std::make_shared<core::domain::Statement>();
  statement->rename(persistedStatementDraft->name);

  return state_.restoreDraft(statement, snapshot.catalog, txDrafts,
                             importMatcherService_, restoredDraftId, 0, this);
}

void ImportWorkflow::rejectImportStart(const QString &errorMessage,
                                         const char *traceMessage) {
  state_.rejectStart(errorMessage);
  observability::reportFlow(
      core::errors::ErrorSeverity::Warning,
      observability::codes::FlowImportRejected,
      observability::origins::workflow::import::kStart, traceMessage);
  emit stateChanged();
  emit importFailed(state_.error());
}

void ImportWorkflow::requestImportCancellation(bool clearQueue,
                                                 const char *origin,
                                                 const char *traceMessage) {
  if (!state_.isRunning())
    return;

  const auto queuedBeforeCancel =
      clearQueue ? state_.queuedFiles() : QStringList{};

  state_.beginCancel(clearQueue);

  if (clearQueue && !queuedBeforeCancel.isEmpty()) {
    for (const auto &queuedPath : queuedBeforeCancel) {
      ImportRunRow row;
      row.logId = generateLogId();
      row.time = currentTimestamp();
      row.type = ui::text::importRuns::typeStatement();
      row.file = queuedPath;
      row.status = ui::text::importRuns::statusCanceled();
      row.message = QStringLiteral("Import canceled before start.");
      row.draftAttached = false;
      runs_->upsertRun(row);
    }
    persistRuns();
  }

  if (jobBridge_)
    jobBridge_->cancelCurrent();
  observability::reportFlow(core::errors::ErrorSeverity::Info,
                            observability::codes::FlowImportCanceled, origin,
                            traceMessage,
                            {{observability::context::kFile,
                              strings::toStdString(state_.currentRunFile())},
                             {observability::context::kQueuedCount,
                              std::to_string(state_.queuedFiles().size())}});
  emit stateChanged();
}

void ImportWorkflow::handleJobEvent(const core::jobs::JobEvent &event) {
  const double progress = clampProgress(event.progress);
  const QString phase = QString::fromStdString(event.message);
  const auto eventState = event.state;
  const QString message = QString::fromStdString(event.message);

  QMetaObject::invokeMethod(
      this,
      [this, progress, phase, eventState, message]() {
        updateProgress(progress, phase);
        if (eventState == core::jobs::JobState::Finished ||
            eventState == core::jobs::JobState::Failed ||
            eventState == core::jobs::JobState::Canceled) {
          onJobTerminal(eventState, message);
        }
      },
      Qt::QueuedConnection);
}

void ImportWorkflow::handleImportCanceled(const QString &now) {
  state_.recordCanceled(now);
  if (!activeRunTerminalHandled_) {
    upsertRunById(activeRunLogId_, ui::text::importRuns::statusCanceled(),
                  ui::text::importPhases::canceled(), false);
    activeRunTerminalHandled_ = true;
  }
  persistRuns();
  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportCanceled,
      observability::origins::workflow::import::kTerminal,
      "Import canceled",
      {{observability::context::kStatus,
        strings::toStdString(ui::text::importRuns::statusCanceled())}});
  emit stateChanged();
  emit importCanceled();
}

void ImportWorkflow::handleImportFailed(const QString &now,
                                          const QString &errorMessage,
                                          const char *traceMessage) {
  state_.recordFailed(now, errorMessage);
  if (!activeRunTerminalHandled_) {
    upsertRunById(activeRunLogId_, ui::text::importRuns::statusFailed(),
                  errorMessage, false);
    activeRunTerminalHandled_ = true;
  }
  persistRuns();
  observability::reportFlow(
      core::errors::ErrorSeverity::Warning,
      observability::codes::FlowImportFailed,
      observability::origins::workflow::import::kTerminal, traceMessage,
      {{observability::context::kError, strings::toStdString(state_.error())}});
  emit stateChanged();
  emit importFailed(state_.error());
}

bool ImportWorkflow::populateDraftFromResult(const QString &now) {
  if (!jobBridge_) {
    handleImportFailed(now, ui::text::workflowErrors::importFailed(),
                       "Import failed: job bridge unavailable");
    return false;
  }

  const auto imported = jobBridge_->present();
  const auto presented =
      importPresenter_ ? importPresenter_->present(imported) : imported;
  if (!presented.data) {
    handleImportFailed(now, ui::text::workflowErrors::importFailed(),
                       "Import failed: missing statement result");
    return false;
  }

  const auto snapshot = stateSnapshotProvider_
                            ? stateSnapshotProvider_()
                            : core::application::workspace::WorkspaceSessionState{};
  const QString draftId =
      activeRunDraftId_.isEmpty() ? activeRunLogId_ : activeRunDraftId_;
  const bool hadVisibleDraft = state_.draft() != nullptr;

  if (!saveImportedDraft(draftId, presented.data, presented.transactions)) {
    handleImportFailed(now, ui::text::workflowErrors::importFailed(),
                       "Import failed: unable to persist draft state");
    return false;
  }

  if (hadVisibleDraft) {
    state_.recordFinished(now);
  } else if (!state_.populateDraft(now, presented.data, snapshot.catalog,
                                   presented.transactions, presented.artifacts,
                                   importMatcherService_, draftId, 0, this)) {
    handleImportFailed(now, ui::text::workflowErrors::importFailed(),
                       "Import failed: unable to create statement draft");
    return false;
  }

  upsertRunById(activeRunLogId_, ui::text::importRuns::statusDraft(),
                QStringLiteral("Draft ready for manual review."), true,
                draftId);
  if (!hadVisibleDraft) {
    activeDraftLogId_ = activeRunLogId_;
  }
  activeRunTerminalHandled_ = true;
  persistRuns();

  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportFinished,
      observability::origins::workflow::import::kTerminal,
      "Import finished",
      {{observability::context::kStatus,
        strings::toStdString(ui::text::importRuns::statusSuccess())},
       {observability::context::kArtifactCount,
        std::to_string(state_.artifactCount())}});
  emit stateChanged();
  emit importFinished();
  return true;
}

bool ImportWorkflow::saveImportedDraft(
    const QString &draftId,
    const std::shared_ptr<core::domain::Statement> &statement,
    const std::vector<core::application::importing::draft::TransactionDraft>
        &transactions) const {
  if (!statementDraftStore_ || !statement || draftId.isEmpty()) {
    return false;
  }

  core::application::importing::draft::StatementDraft draft;
  draft.id = strings::toStdString(draftId);
  QString name = QString::fromStdString(statement->name());
  if (name.trimmed().isEmpty()) {
    const auto sourceFile = state_.currentRunFile();
    name = QFileInfo(sourceFile).baseName();
  }
  draft.name = strings::toStdString(name);
  draft.transactions = transactions;
  statementDraftStore_(draft);
  return true;
}

void ImportWorkflow::cancelImport() {
  requestImportCancellation(
      false, observability::origins::workflow::import::kCancel,
      "Import cancellation requested");
}

void ImportWorkflow::cancelAllImports() {
  requestImportCancellation(
      true, observability::origins::workflow::import::kCancelAll,
      "Cancel-all requested for import queue");
}

void ImportWorkflow::startNextQueuedImport() {
  QString next;
  if (!state_.takeNextQueuedFile(next))
    return;
  emit stateChanged();
  startImportForFile(next);
}

void ImportWorkflow::startStatementImport() {
  if (state_.isRunning() || state_.draft())
    return;

  const auto t = state_.takeSelectedFileForStart();
  if (!t.isEmpty()) {
    emit stateChanged();
    startImportForFile(t);
    return;
  }

  startNextQueuedImport();
}

void ImportWorkflow::startImportForFile(const QString &path) {
  if (state_.isRunning())
    return;
  if (!ensureJobBridge() || !jobBridge_ || !jobBridge_->isAvailable()) {
    rejectImportStart(ui::text::workflowErrors::importWorkflowUnavailable(),
                      "Import start rejected: workflow unavailable");
    return;
  }
  if (path.trimmed().isEmpty()) {
    rejectImportStart(ui::text::workflowErrors::noFileSelected(),
                      "Import start rejected: no file selected");
    return;
  }

  QString runRootQ;
  const auto spec = buildImportSpec(path, runRootQ);

  state_.beginImport(path);
  activeRunLogId_ = generateLogId();
  activeRunDraftId_ = activeRunLogId_;
  activeRunTerminalHandled_ = false;
  upsertRunById(activeRunLogId_, ui::text::importRuns::statusRunning(),
                ui::text::importPhases::starting(), false);
  persistRuns();
  emit stateChanged();

  observability::reportFlow(
      core::errors::ErrorSeverity::Info,
      observability::codes::FlowImportStarted,
      observability::origins::workflow::import::kStart, "Import started",
      {{observability::context::kFile, strings::toStdString(path)},
       {observability::context::kRunRoot, strings::toStdString(runRootQ)},
       {observability::context::kQueuedCount,
        std::to_string(state_.queuedFiles().size())}});

  const bool started = jobBridge_->startStatementImport(
      spec,
      [this](const core::jobs::JobEvent &event) { handleJobEvent(event); });

  if (!started) {
    handleImportFailed(currentTimestamp(),
                       ui::text::workflowErrors::importFailed(),
                       "Import failed: unable to start job");
  }
}

void ImportWorkflow::updateProgress(double p, const QString &phase) {
  static const QRegularExpression re(ui::config::kImportProgressPagePattern);
  state_.updateProgress(p, phase, re);
  emit stateChanged();
}

void ImportWorkflow::onJobTerminal(core::jobs::JobState state,
                                     const QString &message) {
  if ((!state_.isRunning() && !state_.cancelRequested()) ||
      activeRunTerminalHandled_) {
    return;
  }

  const auto now = currentTimestamp();

  if (state == core::jobs::JobState::Canceled || state_.cancelRequested()) {
    if (jobBridge_)
      jobBridge_->clearSubscription();
    handleImportCanceled(now);
    return;
  }

  if (state == core::jobs::JobState::Failed) {
    if (jobBridge_)
      jobBridge_->clearSubscription();
    handleImportFailed(now,
                       message.isEmpty()
                           ? ui::text::workflowErrors::importFailed()
                           : message,
                       "Import failed");
    return;
  }

  const bool populated = populateDraftFromResult(now);
  if (jobBridge_)
    jobBridge_->clearSubscription();
  if (!populated)
    return;

  startNextQueuedImport();
}

void ImportWorkflow::reportException(const char *origin,
                                       std::exception_ptr exception) const {
  if (!errorReporter_)
    return;

  errorReporter_->reportException(core::errors::ErrorSeverity::Error, origin,
                                  exception);
}

} // namespace ui

