#include "persistence/repositories/SqliteContractRepository.h"
#include <sqlite3.h>
#include <stdexcept>

#include "core/models/Contract.h"
#include "persistence/SqliteDb.h"
#include "persistence/Uuid.h"

struct SqliteContractRepository::Impl { std::shared_ptr<SqliteDb> db; };

SqliteContractRepository::SqliteContractRepository(const std::string& dbPath)
    : SqliteContractRepository(std::make_shared<SqliteDb>(dbPath)) {
}

SqliteContractRepository::SqliteContractRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>()) {
    if (!db) throw std::runtime_error("db is null");
    pimpl_->db = std::move(db);
}

SqliteContractRepository::~SqliteContractRepository() = default;

static void insertRelations(sqlite3* db, const std::string& contractId, const Contract& c) {
    {
        const char* sql = "INSERT OR IGNORE INTO contract_actors (contract_id, actor_id) VALUES (?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            for (const std::string& aidStr : c.actorIds) {
                if (aidStr.empty()) continue;
                sqlite3_reset(stmt);
                sqlite3_bind_text(stmt, 1, contractId.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, aidStr.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_step(stmt);
            }
            sqlite3_finalize(stmt);
        }
    }
    {
        const char* sql = "INSERT OR IGNORE INTO contract_properties (contract_id, property_id) VALUES (?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            for (const std::string& pidStr : c.propertyIds) {
                if (pidStr.empty()) continue;
                sqlite3_reset(stmt);
                sqlite3_bind_text(stmt, 1, contractId.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, pidStr.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_step(stmt);
            }
            sqlite3_finalize(stmt);
        }
    }
}

void SqliteContractRepository::addContract(const std::shared_ptr<Contract>& contract) {
    if (!contract) return;
    if (contract->id.empty()) contract->id = persistence::generateUuid();

    const char* sql = "INSERT INTO contracts (id, name, type, description, start_date, end_date, base_price, consumption_price, monthly_advance) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, contract->id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, contract->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, contract->type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, contract->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, contract->startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, contract->endDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 7, contract->basePrice);
    sqlite3_bind_double(stmt, 8, contract->consumptionPrice);
    sqlite3_bind_double(stmt, 9, contract->monthlyAdvance);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        insertRelations(pimpl_->db->handle(), contract->id, *contract);
    }

    sqlite3_finalize(stmt);
}

std::vector<std::shared_ptr<Contract>> SqliteContractRepository::getContracts() const {
    std::vector<std::shared_ptr<Contract>> out;
    const char* sql = "SELECT id, name, type, description, start_date, end_date, base_price, consumption_price, monthly_advance FROM contracts ORDER BY id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* type = sqlite3_column_text(stmt, 2);
        const unsigned char* desc = sqlite3_column_text(stmt, 3);
        const unsigned char* start = sqlite3_column_text(stmt, 4);
        const unsigned char* end = sqlite3_column_text(stmt, 5);
        double base = sqlite3_column_double(stmt, 6);
        double cons = sqlite3_column_double(stmt, 7);
        double adv = sqlite3_column_double(stmt, 8);

        auto c = std::make_shared<Contract>();
        c->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
        c->name = name ? reinterpret_cast<const char*>(name) : std::string();
        c->type = type ? reinterpret_cast<const char*>(type) : std::string();
        c->description = desc ? reinterpret_cast<const char*>(desc) : std::string();
        c->startDate = start ? reinterpret_cast<const char*>(start) : std::string();
        c->endDate = end ? reinterpret_cast<const char*>(end) : std::string();
        c->basePrice = base;
        c->consumptionPrice = cons;
        c->monthlyAdvance = adv;

        {
            const char* rel = "SELECT actor_id FROM contract_actors WHERE contract_id = ? ORDER BY actor_id;";
            sqlite3_stmt* relStmt = nullptr;
            if (sqlite3_prepare_v2(pimpl_->db->handle(), rel, -1, &relStmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(relStmt, 1, c->id.c_str(), -1, SQLITE_TRANSIENT);
                while (sqlite3_step(relStmt) == SQLITE_ROW) {
                    const unsigned char* aidtxt = sqlite3_column_text(relStmt, 0);
                    if (aidtxt) c->actorIds.push_back(reinterpret_cast<const char*>(aidtxt));
                }
                sqlite3_finalize(relStmt);
            }
        }
        {
            const char* rel = "SELECT property_id FROM contract_properties WHERE contract_id = ? ORDER BY property_id;";
            sqlite3_stmt* relStmt = nullptr;
            if (sqlite3_prepare_v2(pimpl_->db->handle(), rel, -1, &relStmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(relStmt, 1, c->id.c_str(), -1, SQLITE_TRANSIENT);
                while (sqlite3_step(relStmt) == SQLITE_ROW) {
                    const unsigned char* pidtxt = sqlite3_column_text(relStmt, 0);
                    if (pidtxt) c->propertyIds.push_back(reinterpret_cast<const char*>(pidtxt));
                }
                sqlite3_finalize(relStmt);
            }
        }

        out.push_back(std::move(c));
    }

    sqlite3_finalize(stmt);
    // Debug: report loaded contracts
    fprintf(stderr, "SqliteContractRepository::getContracts: loaded %zu contracts\n", out.size());
    for (size_t i = 0; i < out.size() && i < 20; ++i) {
        const auto &c = out[i];
        if (!c) continue;
        fprintf(stderr, "  contract[%zu] id='%s' name='%s' type='%s' props=%zu actors=%zu\n", i, c->id.c_str(), c->name.c_str(), c->type.c_str(), c->propertyIds.size(), c->actorIds.size());
    }
    return out;
}

std::vector<std::shared_ptr<Contract>> SqliteContractRepository::getContractsForActor(const std::string& actorId) const {
    std::vector<std::shared_ptr<Contract>> out;
    if (actorId.empty()) return out;

    const char* sql = "SELECT c.id, c.name, c.type, c.description, c.start_date, c.end_date, c.base_price, c.consumption_price, c.monthly_advance FROM contracts c JOIN contract_actors ca ON c.id = ca.contract_id WHERE ca.actor_id = ? ORDER BY c.id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    sqlite3_bind_text(stmt, 1, actorId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* type = sqlite3_column_text(stmt, 2);
        const unsigned char* desc = sqlite3_column_text(stmt, 3);
        const unsigned char* start = sqlite3_column_text(stmt, 4);
        const unsigned char* end = sqlite3_column_text(stmt, 5);
        double base = sqlite3_column_double(stmt, 6);
        double cons = sqlite3_column_double(stmt, 7);
        double adv = sqlite3_column_double(stmt, 8);

        auto c = std::make_shared<Contract>();
        c->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
        c->name = name ? reinterpret_cast<const char*>(name) : std::string();
        c->type = type ? reinterpret_cast<const char*>(type) : std::string();
        c->description = desc ? reinterpret_cast<const char*>(desc) : std::string();
        c->startDate = start ? reinterpret_cast<const char*>(start) : std::string();
        c->endDate = end ? reinterpret_cast<const char*>(end) : std::string();
        c->basePrice = base;
        c->consumptionPrice = cons;
        c->monthlyAdvance = adv;

        out.push_back(std::move(c));
    }

    sqlite3_finalize(stmt);
    return out;
}

std::vector<std::shared_ptr<Contract>> SqliteContractRepository::getContractsForProperty(const std::string& propertyId) const {
    std::vector<std::shared_ptr<Contract>> out;
    if (propertyId.empty()) return out;

    const char* sql = "SELECT c.id, c.name, c.type, c.description, c.start_date, c.end_date, c.base_price, c.consumption_price, c.monthly_advance FROM contracts c JOIN contract_properties cp ON c.id = cp.contract_id WHERE cp.property_id = ? ORDER BY c.id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    sqlite3_bind_text(stmt, 1, propertyId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* type = sqlite3_column_text(stmt, 2);
        const unsigned char* desc = sqlite3_column_text(stmt, 3);
        const unsigned char* start = sqlite3_column_text(stmt, 4);
        const unsigned char* end = sqlite3_column_text(stmt, 5);
        double base = sqlite3_column_double(stmt, 6);
        double cons = sqlite3_column_double(stmt, 7);
        double adv = sqlite3_column_double(stmt, 8);

        auto c = std::make_shared<Contract>();
        c->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
        c->name = name ? reinterpret_cast<const char*>(name) : std::string();
        c->type = type ? reinterpret_cast<const char*>(type) : std::string();
        c->description = desc ? reinterpret_cast<const char*>(desc) : std::string();
        c->startDate = start ? reinterpret_cast<const char*>(start) : std::string();
        c->endDate = end ? reinterpret_cast<const char*>(end) : std::string();
        c->basePrice = base;
        c->consumptionPrice = cons;
        c->monthlyAdvance = adv;

        out.push_back(std::move(c));
    }

    sqlite3_finalize(stmt);
    return out;
}

std::vector<std::string> SqliteContractRepository::getActorIdsForContract(const std::string& contractId) const {
    std::vector<std::string> out;
    if (contractId.empty()) return out;

    const char* sql = "SELECT actor_id FROM contract_actors WHERE contract_id = ? ORDER BY actor_id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    sqlite3_bind_text(stmt, 1, contractId.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* aidtxt = sqlite3_column_text(stmt, 0);
        if (aidtxt) out.push_back(reinterpret_cast<const char*>(aidtxt));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::vector<std::string> SqliteContractRepository::getPropertyIdsForContract(const std::string& contractId) const {
    std::vector<std::string> out;
    if (contractId.empty()) return out;

    const char* sql = "SELECT property_id FROM contract_properties WHERE contract_id = ? ORDER BY property_id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return out;
    sqlite3_bind_text(stmt, 1, contractId.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* pidtxt = sqlite3_column_text(stmt, 0);
        if (pidtxt) out.push_back(reinterpret_cast<const char*>(pidtxt));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<std::shared_ptr<Contract>> SqliteContractRepository::getContractById(const std::string& id) const {
    if (id.empty()) return std::nullopt;

    const char* sql = "SELECT id, name, type, description, start_date, end_date, base_price, consumption_price, monthly_advance FROM contracts WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    const unsigned char* idtxt = sqlite3_column_text(stmt, 0);
    const unsigned char* name = sqlite3_column_text(stmt, 1);
    const unsigned char* type = sqlite3_column_text(stmt, 2);
    const unsigned char* desc = sqlite3_column_text(stmt, 3);
    const unsigned char* start = sqlite3_column_text(stmt, 4);
    const unsigned char* end = sqlite3_column_text(stmt, 5);
    double base = sqlite3_column_double(stmt, 6);
    double cons = sqlite3_column_double(stmt, 7);
    double adv = sqlite3_column_double(stmt, 8);

    auto c = std::make_shared<Contract>();
    c->id = idtxt ? reinterpret_cast<const char*>(idtxt) : std::string();
    c->name = name ? reinterpret_cast<const char*>(name) : std::string();
    c->type = type ? reinterpret_cast<const char*>(type) : std::string();
    c->description = desc ? reinterpret_cast<const char*>(desc) : std::string();
    c->startDate = start ? reinterpret_cast<const char*>(start) : std::string();
    c->endDate = end ? reinterpret_cast<const char*>(end) : std::string();
    c->basePrice = base;
    c->consumptionPrice = cons;
    c->monthlyAdvance = adv;

    sqlite3_finalize(stmt);

    {
        const char* rel = "SELECT actor_id FROM contract_actors WHERE contract_id = ? ORDER BY actor_id;";
        sqlite3_stmt* relStmt = nullptr;
        if (sqlite3_prepare_v2(pimpl_->db->handle(), rel, -1, &relStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(relStmt, 1, c->id.c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(relStmt) == SQLITE_ROW) {
                const unsigned char* aidtxt = sqlite3_column_text(relStmt, 0);
                if (aidtxt) c->actorIds.push_back(reinterpret_cast<const char*>(aidtxt));
            }
            sqlite3_finalize(relStmt);
        }
    }
    {
        const char* rel = "SELECT property_id FROM contract_properties WHERE contract_id = ? ORDER BY property_id;";
        sqlite3_stmt* relStmt = nullptr;
        if (sqlite3_prepare_v2(pimpl_->db->handle(), rel, -1, &relStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(relStmt, 1, c->id.c_str(), -1, SQLITE_TRANSIENT);
            while (sqlite3_step(relStmt) == SQLITE_ROW) {
                const unsigned char* pidtxt = sqlite3_column_text(relStmt, 0);
                if (pidtxt) c->propertyIds.push_back(reinterpret_cast<const char*>(pidtxt));
            }
            sqlite3_finalize(relStmt);
        }
    }

    return c;
}

void SqliteContractRepository::removeContract(const std::string& id) {
    if (id.empty()) return;
    const char* sql = "DELETE FROM contracts WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteContractRepository::updateContract(const std::shared_ptr<Contract>& contract) {
    if (!contract) return;
    if (contract->id.empty()) return;

    const char* sql = "UPDATE contracts SET name = ?, type = ?, description = ?, start_date = ?, end_date = ?, base_price = ?, consumption_price = ?, monthly_advance = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(pimpl_->db->handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, contract->name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, contract->type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, contract->description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, contract->startDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, contract->endDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 6, contract->basePrice);
    sqlite3_bind_double(stmt, 7, contract->consumptionPrice);
    sqlite3_bind_double(stmt, 8, contract->monthlyAdvance);
    sqlite3_bind_text(stmt, 9, contract->id.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    {
        const char* delA = "DELETE FROM contract_actors WHERE contract_id = ?;";
        sqlite3_stmt* delStmt = nullptr;
        if (sqlite3_prepare_v2(pimpl_->db->handle(), delA, -1, &delStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(delStmt, 1, contract->id.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(delStmt);
            sqlite3_finalize(delStmt);
        }
        const char* delP = "DELETE FROM contract_properties WHERE contract_id = ?;";
        if (sqlite3_prepare_v2(pimpl_->db->handle(), delP, -1, &delStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(delStmt, 1, contract->id.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(delStmt);
            sqlite3_finalize(delStmt);
        }
    }

    insertRelations(pimpl_->db->handle(), contract->id, *contract);
}

void SqliteContractRepository::upsertContract(const std::shared_ptr<Contract>& contract) {
    if (!contract) return;
    if (contract->id.empty()) { addContract(contract); return; }
    updateContract(contract);
    if (pimpl_ && pimpl_->db && sqlite3_changes(pimpl_->db->handle()) == 0) {
        addContract(contract);
    }
}

void SqliteContractRepository::clearContracts() {
    char* err = nullptr;
    sqlite3_exec(pimpl_->db->handle(), "DELETE FROM contracts;", nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}
