#include "pch.h"
#include "models/Header.h"

Header::Header(const std::string& name, int hpos, int vpos, int xmin, int xmax)
    : name(name), hpos(hpos), vpos(vpos), xmin(xmin), xmax(xmax) {
}

int Header::getHpos() const { return hpos; }
int Header::getVpos() const { return vpos; }
const std::string& Header::getName() const { return name; }
int Header::getXmin() const { return xmin; }
int Header::getXmax() const { return xmax; }
void Header::setXmin(int x) { xmin = x; }
void Header::setXmax(int x) { xmax = x; }