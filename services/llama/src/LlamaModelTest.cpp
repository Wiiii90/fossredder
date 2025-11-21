#include "llama/LlamaEngine.h"
#include "llama/prompts/Prompts.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <unordered_map>
#include <regex>

// Extrahiert Tupel im Format (Feldname, Wert) aus dem LLM-Output und speichert sie in einer Hashmap
std::unordered_map<std::string, std::string> parseTuplesToMap(const std::string& response) {
    std::unordered_map<std::string, std::string> result;
    std::regex tupleRegex(R"(\(\s*([^)]+?)\s*,\s*([^)]+?)\s*\))");
    std::smatch match;
    std::string::const_iterator searchStart(response.cbegin());
    while (std::regex_search(searchStart, response.cend(), match, tupleRegex)) {
        if (match.size() == 3) {
            result[match[1].str()] = match[2].str();
        }
        searchStart = match.suffix().first;
    }
    return result;
}

int main(int argc, char* argv[]) {
    std::string modelPath = argc > 1 ? argv[1] : "C:/coding/fossredder/res/llms/em_german_7b_v01.Q5_K_M.gguf";
    std::string ocrText = "Alte Leipziger Versicherung Aktieng esellschaft Vers-Nr.OO-O31-103192 01.2025-Gewer bliche Sachversicherung End-to-End-Ref.: Beleg-Nr.312507351158 Mandatsref: 95283887865 Gläubiger-ID: DE1422200000082458 SEPA-BASISLASTSCHRIFT wiederholend";

    int maxTokensBereinigung = 256;
    int maxTokensExtraktion = 512;
    int top_k = argc > 3 ? std::atoi(argv[3]) : 50;
    float top_p = argc > 4 ? std::atof(argv[4]) : 0.3f;
    float temperature = argc > 5 ? std::atof(argv[5]) : 0.3f;
    float repeat_penalty = argc > 6 ? std::atof(argv[6]) : 0.8f ;

    try {
        LlamaEngine engine(modelPath);

        std::string cleanSystem = llama_prompts::cleaningPromptSystem;
        std::string cleanUser = llama_prompts::cleaningPromptUser + ocrText;
        std::string cleanPrompt = cleanSystem + "\n" + cleanUser + "\n" + "Bereinigter OCR-Text:";

        std::cout << "Prompt an LLM (Bereinigung):\n" << cleanPrompt << std::endl;
        std::string cleanedText = engine.runPrompt(cleanPrompt, maxTokensBereinigung, top_k, top_p, temperature, repeat_penalty);
        std::cout << cleanedText << std::endl;

        engine.resetContext();

        std::vector<std::pair<std::string, std::vector<std::string>>> beispiele = {
            {"Max Mustermann Gläubiger: DE1234567890123456 End-to-End-Ref.: Beleg-Nr.123123123123",
                {"(Empfänger, Max Mustermann)", "(Gläubiger-ID, DE1234567890123456)", "(End-to-End-Ref, Beleg-Nr. 123123123123)", "(Beleg-Nr., 123123123123)"}},
            {"Musterfirma GmbH Vers-Nr.1234567890 DE9876543210987654 Text Mandatsref: 654321",
                {"(Empfänger, Musterfirma GmbH Vers-Nr.1234567890 DE9876543210987654 Text)", "(Vers-Nr., 1234567890)", "(Gläubiger-ID, DE9876543210987654)", "(Mandatsreferenz, 654321)"}}
        };

        std::string system = llama_prompts::extractionPromptSystem;
        std::string user = llama_prompts::extractionPromptUserHeader;
        for (size_t i = 0; i < beispiele.size(); ++i) {
            user += "Beispiel " + std::to_string(i + 1) + ": " + beispiele[i].first + "\n";
            for (const auto& tupel : beispiele[i].second) {
                user += tupel + "\n";
            }
            user += "\n";
        }
        user += "Banktransaktionsdetails: " + cleanedText;
        std::string prompt = system + "\n" + user + "\n" + "Extrahierte Tupel:\n";

        std::cout << "Prompt an LLM (Feldextraktion):\n" << prompt << std::endl;
        std::string response = engine.runPrompt(prompt, maxTokensExtraktion, top_k, top_p, temperature, repeat_penalty);
        std::cout << response << std::endl;

        // Tupel in Hashmap speichern und ausgeben
        auto fieldMap = parseTuplesToMap(response);
        std::cout << "\nExtrahierte Felder als Key-Value-Paare:" << std::endl;
        for (const auto& [key, value] : fieldMap) {
            std::cout << key << " => " << value << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Fehler: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
