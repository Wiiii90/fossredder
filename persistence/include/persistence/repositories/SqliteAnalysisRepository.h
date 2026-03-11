#pragma once

#include "core/repositories/IAnalysisRepository.h"
#include "persistence/ISqlConnectionProvider.h"

#include <memory>
#include <string>

class SqliteDb;

class SqliteAnalysisRepository : public IAnalysisRepository, public ISqlConnectionProvider {
public:
    explicit SqliteAnalysisRepository(const std::string& dbPath);
    explicit SqliteAnalysisRepository(std::shared_ptr<SqliteDb> db);
    ~SqliteAnalysisRepository() override;

    void addAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) override;
    std::vector<std::shared_ptr<core::domain::Analysis>> getAnalyses() const override;
    std::optional<std::shared_ptr<core::domain::Analysis>> getAnalysisById(const std::string& id) const override;
    void removeAnalysis(const std::string& id) override;
    void updateAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) override;

    void upsertAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) override;
    void clearAnalyses() override;

    sqlite3* sqliteHandle() const noexcept override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
