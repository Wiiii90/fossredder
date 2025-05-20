#pragma once
#include "pch.h"

class Header {
public:
    Header(const std::string& name, int hpos, int vpos, int xmin = -1, int xmax = -1);
    int getHpos() const;
    int getVpos() const;
    const std::string& getName() const;
    int getXmin() const;
    int getXmax() const;
    void setXmin(int x);
    void setXmax(int x);
private:
    std::string name;
    int hpos;
    int vpos;
    int xmin;
    int xmax;
};