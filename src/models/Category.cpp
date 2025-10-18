#include "models/Category.h"

Category::Category(const std::string& name, bool isAllocatable, const std::string& description)
	: name(name), isAllocatable(isAllocatable), description(description) {
}