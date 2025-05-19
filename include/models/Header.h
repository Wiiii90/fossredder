#pragma once
#include <string>

class Header {
public:
    Header(const std::string& name, int hpos, int vpos);
    const std::string& getName() const;
    int getHpos() const;
    int getVpos() const;
private:
    std::string name;
    int hpos;
    int vpos;
};
