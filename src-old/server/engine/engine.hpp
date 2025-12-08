#pragma once
#include "networker.hpp"

class Engine {
public:
    Networker networker;

    void Init() {
        networker.Init();
    }
    
    void Close() {
        networker.Close();
    }
};