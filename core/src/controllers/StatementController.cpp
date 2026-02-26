#include "core/pch.h"
#include "core/controllers/StatementController.h"
#include "core/import/IImportStatement.h"
#include "core/models/Statement.h"
#include "core/models/Transaction.h"
#include "core/controllers/AppStateController.h"
#include <iostream>
#include <algorithm>
#include <cctype>

static std::string lowerTrim(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) out.push_back(static_cast<char>(std::tolower(c)));
    // trim
    size_t i = 0; while (i < out.size() && std::isspace(static_cast<unsigned char>(out[i]))) ++i;
    size_t j = out.size(); while (j > i && std::isspace(static_cast<unsigned char>(out[j-1]))) --j;
    if (i == 0 && j == out.size()) return out;
    return out.substr(i, j - i);
}

static bool nearlyEqual(double a, double b, double eps = 0.01) {
    return std::abs(a - b) <= eps;
}

static bool txExists(const std::vector<std::shared_ptr<Transaction>>& list, const std::shared_ptr<Transaction>& tx) {
    if (!tx) return false;
    const auto name = lowerTrim(tx->name);
    const auto bdate = tx->bookingDate;
    const auto valuta = tx->valuta;
    const double amount = tx->amount;

    for (const auto& e : list) {
        if (!e) continue;
        // if statementId present on existing tx and new tx has same statementId -> likely same
        if (!e->statementId.empty() && !tx->statementId.empty() && e->statementId == tx->statementId && nearlyEqual(e->amount, tx->amount)) return true;

        // match by normalized name + booking date + amount approx
        if (!lowerTrim(e->name).empty() && lowerTrim(e->name) == name
            && e->bookingDate == bdate
            && nearlyEqual(e->amount, amount)) return true;

        // fallback: match by bookingDate + valuta + amount
        if (!bdate.empty() && e->bookingDate == bdate && e->valuta == valuta && nearlyEqual(e->amount, amount)) return true;

        // consider metadata/actorProposal similarity
        if (!e->metadata.empty() && !tx->metadata.empty() && e->metadata == tx->metadata && nearlyEqual(e->amount, amount)) return true;
        if (!e->actorProposal.empty() && !tx->actorProposal.empty() && lowerTrim(e->actorProposal) == lowerTrim(tx->actorProposal) && nearlyEqual(e->amount, amount)) return true;
    }
    return false;
}

std::shared_ptr<Statement> StatementController::importStatement(const std::string& filePath,
                                                               const std::string& runRoot,
                                                               const std::string& runIdPrefix,
                                                               std::function<void(double, const std::string&)> progressCallback,
                                                               std::shared_ptr<std::atomic<bool>> cancelFlag,
                                                               core::jobs::Scheduler* scheduler,
                                                               core::jobs::SlotLimiter* ocrLimiter,
                                                               std::string jobId) {
    if (!std::filesystem::exists(filePath)) {
        throw std::runtime_error("PDF datei existiert nicht: " + filePath);
    }

    if (!importService_) {
        throw std::runtime_error("No import service configured");
    }

    if (runRoot.empty()) {
        throw std::runtime_error("No import runRoot configured");
    }

    try {
        std::clog << "StatementController::importStatement - starting import: " << filePath << std::endl;
    } catch (...) {}

    ImportRequest req;
    req.sourcePath = filePath;
    req.runRoot = runRoot;
    req.runIdPrefix = runIdPrefix;
    req.jobId = std::move(jobId);
    req.progressCallback = std::move(progressCallback);
    req.cancelFlag = cancelFlag;
    req.scheduler = scheduler;
    req.ocrLimiter = ocrLimiter;
    ImportResult res = importService_->importStatement(req);

    try {
        std::clog << "StatementController::importStatement - import completed for: " << filePath
                  << ", data=" << (res.data ? "present" : "null")
                  << ", artifacts=" << res.artifacts.size() << std::endl;
    } catch (...) {}

    if (!res.data) {
        throw std::runtime_error("Extraction failed");
    }

    // Notify optional callback about extracted transactions (if any)
    try {
        if (onTransactionsExtracted_ && res.data) {
            onTransactionsExtracted_(res.data->transactions);
        }
    } catch (...) {
        // ensure callback exceptions do not break import
        try { std::clog << "StatementController::importStatement - onTransactionsExtracted_ callback threw" << std::endl; } catch (...) {}
    }

    // Persistence intentionally not performed here. StatementController reports parsed data
    // via `onTransactionsExtracted_` and returns the parsed statement. Higher layers (UI)
    // are responsible to decide if/when to persist into AppState atomically on the main thread.

    // Provide an optional fuzzy duplicate helper via utils for callers. Not used here directly.

    return res.data;
}