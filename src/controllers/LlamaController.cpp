#include "controllers/LlamaController.h"
#include <QDebug>
#include <regex>
#include "util/OutputParser.h"
#include "llama/prompts/Prompts.h"

void LlamaController::enrichTransactions(LlamaEngine& engine, std::vector<std::shared_ptr<Transaction>>& transactions) {
    for (size_t i = 0; i < transactions.size(); ++i) {
        auto& tx = transactions[i];
        if (!tx) continue;

        engine.resetContext();

        std::string cleanSystem = llama_prompts::cleaningPromptSystem;
        std::string cleanUser = llama_prompts::cleaningPromptUser + tx->details;
        std::string cleanPrompt = cleanSystem + cleanUser;

        qDebug() << "Bereinigungsprompt für Transaktion" << i << ":" << QString::fromStdString(cleanPrompt);

        std::string cleanedText;
        try {
            cleanedText = engine.runPrompt(cleanPrompt, 256, 50, 0.3f, 0.3f, 0.8f);
        } catch (const std::exception& e) {
            qDebug() << "Bereinigung Exception:" << e.what();
            continue;
        }

        qDebug() << "Bereinigter Text für Transaktion" << i << ":" << QString::fromStdString(cleanedText);

        // Verbesserte heuristische Extraktion der Felder aus cleanedText
        std::unordered_map<std::string, std::string> fieldMap;
        std::string text = cleanedText;
        text.erase(0, text.find_first_not_of(" \t\r\n"));
        text.erase(text.find_last_not_of(" \t\r\n") + 1);

        // Verbesserte Regex: alles bis zum Doppelpunkt
        std::regex fieldRegex(R"(([^:]+):)");
        std::vector<std::pair<std::string, size_t>> fieldPositions;
        auto begin = std::sregex_iterator(text.begin(), text.end(), fieldRegex);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            fieldPositions.emplace_back((*it)[1].str(), static_cast<size_t>(it->position()));
        }

        if (!fieldPositions.empty()) {
            // Empfänger: alles vor dem ersten Feldnamen mit Doppelpunkt
            std::string empfaenger = text.substr(0, fieldPositions[0].second);
            empfaenger.erase(0, empfaenger.find_first_not_of(" \t\r\n"));
            empfaenger.erase(empfaenger.find_last_not_of(" \t\r\n") + 1);
            fieldMap["Empfänger"] = empfaenger;
            for (size_t j = 0; j < fieldPositions.size(); ++j) {
                size_t keyStart = fieldPositions[j].second;
                size_t keylen = fieldPositions[j].first.length();
                size_t valueStart = keyStart + keylen + 1; // +1 für ':'
                size_t valueEnd = (j + 1 < fieldPositions.size()) ? fieldPositions[j + 1].second : text.length();
                std::string value = text.substr(valueStart, valueEnd - valueStart);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                fieldMap[fieldPositions[j].first] = value;
            }
        } else {
            // Kein Feld gefunden, alles als Empfänger
            std::string empfaenger = text;
            empfaenger.erase(0, empfaenger.find_first_not_of(" \t\r\n"));
            empfaenger.erase(empfaenger.find_last_not_of(" \t\r\n") + 1);
            fieldMap["Empfänger"] = empfaenger;
        }

        // Verbesserte Konsolenausgabe
        qDebug() << "Extrahierte Felder für Transaktion" << i << ":";
        for (const auto& kv : fieldMap) {
            qDebug() << "Feldname:" << QString::fromStdString(kv.first) << "=> Wert:" << QString::fromStdString(kv.second);
        }
        // Hier kann das Mapping weiterverwendet werden (z.B. in Transaction speichern)
    }
}
