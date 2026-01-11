#include "ui/state/EditSession.h"
#include <QUuid>
#include <QDebug>

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
    // emit requestApply; controller will perform actual apply and then emit saved()
    qDebug() << "EditSession::apply() sessionId=" << sessionId_ << " txId=" << txId_;
    // mark as saving so UI can show a pending state
    setSaving(true);
    emit requestApply(sessionId_);
}

void EditSession::cancel() {
    // reset fields and mark clean
    qDebug() << "EditSession::cancel() sessionId=" << sessionId_ << " txId=" << txId_;
    resetFromSnapshot();
    dirty_ = false;
    emit dirtyChanged();
}

void EditSession::startSession(const QString& txId) {
    // If we're already started for this txId, do nothing.
    if (txId_ == txId) return;

    // New session for a different transaction: generate a fresh session id
    sessionId_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
    qDebug() << "EditSession::startSession txId=" << txId << " sessionId=" << sessionId_;
    txId_ = txId;
    // When starting new session, clear dirty state
    dirty_ = false;
    saving_ = false;
    emit dirtyChanged();
    emit savingChanged();
}

void EditSession::resetFromSnapshot() {
    // Snapshot population is done by caller via setters or direct friend access
    // For now do nothing.
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
    // Populate fields without marking session dirty
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
    qDebug() << "EditSession::setName sessionId=" << sessionId_ << " txId=" << txId_ << " name=" << v;
}
void EditSession::setBookingDate(const QString& v) {
    if (bookingDate_ == v) return;
    bookingDate_ = v;
    markDirty();
    emit changed();
    qDebug() << "EditSession::setBookingDate sessionId=" << sessionId_ << " txId=" << txId_ << " bookingDate=" << v;
}
void EditSession::setValuta(const QString& v) {
    if (valuta_ == v) return;
    valuta_ = v;
    markDirty();
    emit changed();
    qDebug() << "EditSession::setValuta sessionId=" << sessionId_ << " txId=" << txId_ << " valuta=" << v;
}
void EditSession::setAmount(double v) {
    if (qFuzzyCompare(amount_ + 1, v + 1)) return;
    amount_ = v;
    markDirty();
    emit changed();
    qDebug() << "EditSession::setAmount sessionId=" << sessionId_ << " txId=" << txId_ << " amount=" << v;
}
void EditSession::setDescription(const QString& v) {
    if (description_ == v) return;
    description_ = v;
    markDirty();
    emit changed();
    qDebug() << "EditSession::setDescription sessionId=" << sessionId_ << " txId=" << txId_ << " description=" << v;
}
void EditSession::setStatementId(const QString& v) {
    if (statementId_ == v) return;
    statementId_ = v;
    markDirty();
    emit changed();
    qDebug() << "EditSession::setStatementId sessionId=" << sessionId_ << " txId=" << txId_ << " statementId=" << v;
}
void EditSession::setActorId(const QString& v) {
    if (actorId_ == v) return;
    actorId_ = v;
    markDirty();
    emit changed();
    qDebug() << "EditSession::setActorId sessionId=" << sessionId_ << " txId=" << txId_ << " actorId=" << v;
}
void EditSession::setActorProposal(const QString& v) {
    if (actorProposal_ == v) return;
    actorProposal_ = v;
    markDirty();
    emit changed();
    qDebug() << "EditSession::setActorProposal sessionId=" << sessionId_ << " txId=" << txId_ << " actorProposal=" << v;
}
void EditSession::setPropertyIds(const QStringList& v) {
    if (propertyIds_ == v) return;
    propertyIds_ = v;
    markDirty();
    emit changed();
    qDebug() << "EditSession::setPropertyIds sessionId=" << sessionId_ << " txId=" << txId_ << " propertyIds=" << v;
}
void EditSession::setAllocatable(bool v) {
    if (allocatable_ == v) return;
    allocatable_ = v;
    markDirty();
    emit changed();
    qDebug() << "EditSession::setAllocatable sessionId=" << sessionId_ << " txId=" << txId_ << " allocatable=" << v;
}
void EditSession::setStatus(int v) {
    if (status_ == v) return;
    status_ = v;
    markDirty();
    emit changed();
    qDebug() << "EditSession::setStatus sessionId=" << sessionId_ << " txId=" << txId_ << " status=" << v;
}
void EditSession::setSaving(bool s) {
    if (saving_ == s) return;
    saving_ = s;
    emit savingChanged();
    qDebug() << "EditSession::setSaving(" << s << ") sessionId=" << sessionId_ << " txId=" << txId_;
}

void EditSession::clearDirty() {
    if (!dirty_) return;
    dirty_ = false;
    emit dirtyChanged();
    qDebug() << "EditSession::clearDirty sessionId=" << sessionId_ << " txId=" << txId_;
}

void EditSession::markDirty() {
    if (!dirty_) {
        dirty_ = true;
        emit dirtyChanged();
        qDebug() << "EditSession::markDirty sessionId=" << sessionId_ << " txId=" << txId_;
    }
}

void EditSession::setExpectedVersion(int v) {
    expectedVersion_ = v;
    emit changed();
    qDebug() << "EditSession::setExpectedVersion sessionId=" << sessionId_ << " txId=" << txId_ << " expectedVersion=" << v;
}

bool EditSession::hasPropertyId(const QString& id) const {
    bool found = propertyIds_.contains(id);
    qDebug() << "EditSession::hasPropertyId(" << id << ") ->" << found << "sessionId=" << sessionId_;
    return found;
}

QStringList EditSession::getPropertyIds() const { return propertyIds_; }
