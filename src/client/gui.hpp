#pragma once
#include "imgui.h"
#include <string>

class GUI {
private:
    

public:
    static GUI &GetInstance();

    void Init();
    void Update();
    void Close();
};