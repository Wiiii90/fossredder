/**
 * @file core/include/core/repositories/IAnalysisRepository.h
 * @brief Repository interface for core::domain::Analysis persistence operations.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace core::domain {
class Analysis;
}

class IAnalysisRepository {
public:
    virtual ~IAnalysisRepository() = default;

    /**
     * @brief Insert a new analysis into the repository.
     * @param analysis Shared pointer to the core::domain::Analysis to add.
     */
    virtual void addAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) = 0;

    /**
     * @brief Retrieve all stored analyses.
     * @return All analyses stored in the repository.
     */
    virtual std::vector<std::shared_ptr<core::domain::Analysis>> getAnalyses() const = 0;

    /**
     * @brief Retrieve an analysis by its identifier.
     * @param id core::domain::Analysis identifier string.
     * @return The analysis with the requested identifier, if found.
     */
    virtual std::optional<std::shared_ptr<core::domain::Analysis>> getAnalysisById(const std::string& id) const = 0;

    /**
     * @brief Remove an analysis identified by id from the repository.
     * @param id core::domain::Analysis identifier to remove.
     */
    virtual void removeAnalysis(const std::string& id) = 0;

    /**
     * @brief Update an existing analysis record in the repository.
     * @param analysis Shared pointer to the core::domain::Analysis with updated fields.
     */
    virtual void updateAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) = 0;

    /**
     * @brief Upsert an analysis: insert or update depending on existence.
     * @param analysis Shared pointer to the core::domain::Analysis to upsert.
     */
    virtual void upsertAnalysis(const std::shared_ptr<core::domain::Analysis>& analysis) = 0;

    /**
     * @brief Remove all analyses from the repository.
     */
    virtual void clearAnalyses() = 0;
};
