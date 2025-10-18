#include "controllers/CsvImportController.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

std::vector<std::vector<std::string>> CsvImportController::importCsv(const std::string& filePath, char delimiter) {
	std::vector<std::vector<std::string>> result;
	std::ifstream file(filePath);
	std::string line;
	while (std::getline(file, line)) {
		std::vector<std::string> row;
		std::stringstream ss(line);
		std::string cell;
		while (std::getline(ss, cell, delimiter)) {
			row.push_back(cell);
		}
		result.push_back(row);
	}
	return result;
}