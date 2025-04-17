#include <iostream>
#include <string>
#include <vector>

// Define a basic Property structure for demonstration
struct Property {
    std::string name;
    std::string address;
    double monthlyRent;
};

// Define a basic Tenant structure for demonstration
struct Tenant {
    std::string name;
    std::string contactInfo;
};

// Function to create a new property
Property createProperty() {
    Property prop;
    std::cout << "Enter property name: ";
    std::getline(std::cin, prop.name);
    std::cout << "Enter property address: ";
    std::getline(std::cin, prop.address);
    std::cout << "Enter monthly rent: ";
    std::cin >> prop.monthlyRent;
    std::cin.ignore(); // Clear the newline character from the input buffer
    return prop;
}

// Function to create a new tenant
Tenant createTenant() {
    Tenant tenant;
    std::cout << "Enter tenant name: ";
    std::getline(std::cin, tenant.name);
    std::cout << "Enter tenant contact info: ";
    std::getline(std::cin, tenant.contactInfo);
    return tenant;
}

// Main function
int main() {
    std::cout << "Welcome to FossRedder!" << std::endl;

    std::vector<Property> properties;
    std::vector<Tenant> tenants;

    while (true) {
        std::cout << "\nMenu:\n";
        std::cout << "1. Add a new property\n";
        std::cout << "2. Add a new tenant\n";
        std::cout << "3. View all properties\n";
        std::cout << "4. View all tenants\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore(); // Clear the newline character from the input buffer

        switch (choice) {
        case 1: {
            Property newProperty = createProperty();
            properties.push_back(newProperty);
            std::cout << "Property added successfully!\n";
            break;
        }
        case 2: {
            Tenant newTenant = createTenant();
            tenants.push_back(newTenant);
            std::cout << "Tenant added successfully!\n";
            break;
        }
        case 3: {
            std::cout << "\nProperties:\n";
            for (const auto& property : properties) {
                std::cout << "Name: " << property.name
                    << ", Address: " << property.address
                    << ", Monthly Rent: $" << property.monthlyRent << "\n";
            }
            break;
        }
        case 4: {
            std::cout << "\nTenants:\n";
            for (const auto& tenant : tenants) {
                std::cout << "Name: " << tenant.name
                    << ", Contact Info: " << tenant.contactInfo << "\n";
            }
            break;
        }
        case 5:
            std::cout << "Exiting the application. Goodbye!\n";
            return 0;
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }
    }

    return 0;
}
