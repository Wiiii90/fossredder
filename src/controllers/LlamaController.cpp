#include "controllers/LlamaController.h"
#include <QDebug>
#include "util/OutputParser.h"
#include "llama/prompts/Prompts.h"

void LlamaController::enrichTransactions(LlamaEngine& engine, std::vector<std::shared_ptr<Transaction>>& transactions) {
    for (size_t i = 0; i < transactions.size(); ++i) {
        auto& tx = transactions[i];
        if (!tx) continue;

        // Kontext vor Bereinigungsprompt zurücksetzen
        engine.resetContext();

        std::string cleanSystem = llama_prompts::cleaningPromptSystem;
        std::string cleanUser = llama_prompts::cleaningPromptUser + tx->details + "\nBetrag: " + tx->amountText;
        std::string cleanPrompt = cleanSystem + "\n" + cleanUser + "\nBereinigter OCR-Text:";

        qDebug() << "Bereinigungsprompt für Transaktion" << i << ":" << QString::fromStdString(cleanPrompt);

        std::string cleanedText;
        try {
            cleanedText = engine.runPrompt(cleanPrompt, 256, 50, 0.3f, 0.3f, 0.8f);
        } catch (const std::exception& e) {
            qDebug() << "Bereinigung Exception:" << e.what();
            continue;
        }

        qDebug() << "Bereinigter Text für Transaktion" << i << ":" << QString::fromStdString(cleanedText);

        // Kontext vor Extraktionsprompt zurücksetzen
        engine.resetContext();

        std::vector<std::pair<std::string, std::vector<std::string>>> examples = {
            {"Max Mustermann Gläubiger: DE1234567890123456 End-to-End-Ref.: Beleg-Nr.123123123123",
                {"(Empfänger, Max Mustermann)", "(Gläubiger-ID, DE1234567890123456)", "(End-to-End-Ref, Beleg-Nr. 123123123123)", "(Beleg-Nr., 123123123123)"}},
            {"Musterfirma GmbH Vers-Nr.1234567890 DE9876543210987654 Text Mandatsref: 654321",
                {"(Empfänger, Musterfirma GmbH Vers-Nr.1234567890 DE9876543210987654 Text)", "(Vers-Nr., 1234567890)", "(Gläubiger-ID, DE9876543210987654)", "(Mandatsreferenz, 654321)"}}
        };
        std::string system = llama_prompts::extractionPromptSystem;
        std::string user = llama_prompts::extractionPromptUserHeader;
        for (size_t j = 0; j < examples.size(); ++j) {
            user += "Beispiel " + std::to_string(j + 1) + ": " + examples[j].first + "\n";
            for (const auto& tupel : examples[j].second) {
                user += tupel + "\n";
            }
            user += "\n";
        }
        user += "Banktransaktionsdetails: " + cleanedText;
        std::string prompt = system + "\n" + user + "\nExtrahierte Tupel:\n";

        qDebug() << "Extraktionsprompt für Transaktion" << i << ":" << QString::fromStdString(prompt);

        std::string response;
        try {
            response = engine.runPrompt(prompt, 512, 50, 0.3f, 0.3f, 0.8f);
        } catch (const std::exception& e) {
            qDebug() << "Extraktion Exception:" << e.what();
            continue;
        }

        qDebug() << "Extraktionsoutput für Transaktion" << i << ":" << QString::fromStdString(response);

        auto fieldMap = OutputParser::parseTuplesToMap(response);
        qDebug() << "Extrahierte Felder für Transaktion" << i << ":";
        for (const auto& kv : fieldMap) {
            qDebug() << QString::fromStdString(kv.first) << "=>" << QString::fromStdString(kv.second);
        }
    }
}
