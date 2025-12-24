#pragma once

#include <string>
#include <vector>

class Actor;
class Property;

class Contract {
public:
    Contract() = default;

    std::string id;
    std::string name;
    std::string type;
    std::string description;

    std::string startDate;
    std::string endDate;

    double basePrice = 0.0;
    double consumptionPrice = 0.0;
    double monthlyAdvance = 0.0;

    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;

    std::vector<Actor*> actors;
    std::vector<Property*> properties;

    void addActor(Actor* actor);
    bool removeActor(Actor* actor);

    void addProperty(Property* property);
    bool removeProperty(Property* property);
};
