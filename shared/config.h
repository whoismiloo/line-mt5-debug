#pragma once
#include <toml.hpp>

struct Config {
    bool terminal;
    bool camera;
    bool terminalProxy;
    
    bool bana;
    char accessCode[24];
    char chipId[36];

    bool input;
    bool inputBackground;

    bool touch;
    
    void load();
    void save();
};

extern Config config;