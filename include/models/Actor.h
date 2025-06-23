#pragma once
#include <string>

class Actor {
public:
    std::string name;
    std::string type; // z.B. "Vermieter", "Mieter", "Dienstleister" etc.
    std::string description;

    Actor(const std::string& name, const std::string& type = "", const std::string& description = "");
};