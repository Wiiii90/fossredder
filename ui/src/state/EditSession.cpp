#include "ui/state/EditSession.h"
#include <QUuid>

EditSession::EditSession(QObject* parent) : QObject(parent) {
    sessionId_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString EditSession::sessionId() const { return sessionId_; }
QString EditSession::txId() const { return txId_; }

QString EditSession::name() const { return name_; }
QString EditSession::bookingDate() const { return bookingDate_; }
QString EditSession::valuta() const { return valuta_; }
double EditSession::amount() const { return amount_; }
QString EditSession::description() const { return description_; }
QString EditSession::statementId() const { return statementId_; }
QString EditSession::actorId() const { return actorId_; }
QString EditSession::actorProposal() const { return actorProposal_; }
QStringList EditSession::propertyIds() const { return propertyIds_; }
bool EditSession::allocatable() const { return allocatable_; }
int EditSession::status() const { return status_; }

bool EditSession::hasUnsavedChanges() const { return dirty_; }
bool EditSession::saving() const { return saving_; }
int EditSession::expectedVersion() const { return expectedVersion_; }

void EditSession::apply() {
    setSaving(true);
    emit requestApply(sessionId_);
}

void EditSession::cancel() {
    resetFromSnapshot();
    dirty_ = false;
    emit dirtyChanged();
}

void EditSession::startSession(const QString& txId) {
    if (txId_ == txId) return;

    sessionId_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
    txId_ = txId;

    dirty_ = false;
    saving_ = false;
    emit dirtyChanged();
    emit savingChanged();
}

void EditSession::resetFromSnapshot() {
    emit changed();
}

void EditSession::loadSnapshot(const QString& name,
                               const QString& bookingDate,
                               double amount,
                               const QString& description,
                               const QString& statementId,
                               const QString& actorId,
                               const QString& actorProposal,
                               const QStringList& propertyIds,
                               bool allocatable,
                               int status,
                               int expectedVersion)
{
    name_ = name;
    bookingDate_ = bookingDate;
    amount_ = amount;
    description_ = description;
    statementId_ = statementId;
    actorId_ = actorId;
    actorProposal_ = actorProposal;
    propertyIds_ = propertyIds;
    allocatable_ = allocatable;
    status_ = status;
    expectedVersion_ = expectedVersion;
    dirty_ = false;
    emit changed();
}

void EditSession::setName(const QString& v) {
    if (name_ == v) return;
    name_ = v;
    markDirty();
    emit changed();
}
void EditSession::setBookingDate(const QString& v) {
    if (bookingDate_ == v) return;
    bookingDate_ = v;
    markDirty();
    emit changed();
}
void EditSession::setValuta(const QString& v) {
    if (valuta_ == v) return;
    valuta_ = v;
    markDirty();
    emit changed();
}
void EditSession::setAmount(double v) {
    if (qFuzzyCompare(amount_ + 1, v + 1)) return;
    amount_ = v;
    markDirty();
    emit changed();
}
void EditSession::setDescription(const QString& v) {
    if (description_ == v) return;
    description_ = v;
    markDirty();
    emit changed();
}
void EditSession::setStatementId(const QString& v) {
    if (statementId_ == v) return;
    statementId_ = v;
    markDirty();
    emit changed();
}
void EditSession::setActorId(const QString& v) {
    if (actorId_ == v) return;
    actorId_ = v;
    markDirty();
    emit changed();
}
void EditSession::setActorProposal(const QString& v) {
    if (actorProposal_ == v) return;
    actorProposal_ = v;
    markDirty();
    emit changed();
}
void EditSession::setPropertyIds(const QStringList& v) {
    if (propertyIds_ == v) return;
    propertyIds_ = v;
    markDirty();
    emit changed();
}
void EditSession::setAllocatable(bool v) {
    if (allocatable_ == v) return;
    allocatable_ = v;
    markDirty();
    emit changed();
}
void EditSession::setStatus(int v) {
    if (status_ == v) return;
    status_ = v;
    markDirty();
    emit changed();
}
void EditSession::setSaving(bool s) {
    if (saving_ == s) return;
    saving_ = s;
    emit savingChanged();
}

void EditSession::clearDirty() {
    if (!dirty_) return;
    dirty_ = false;
    emit dirtyChanged();
}

void EditSession::markDirty() {
    if (!dirty_) {
        dirty_ = true;
        emit dirtyChanged();
    }
}

void EditSession::setExpectedVersion(int v) {
    expectedVersion_ = v;
    emit changed();
}

bool EditSession::hasPropertyId(const QString& id) const {
    return propertyIds_.contains(id);
}

QStringList EditSession::getPropertyIds() const { return propertyIds_; }
