#include "models/Header.h"

Header::Header(const std::string& name, int hpos, int vpos)
    : name(name), hpos(hpos), vpos(vpos) {
}

const std::string& Header::getName() const { return name; }
int Header::getHpos() const { return hpos; }
int Header::getVpos() const { return vpos; }
