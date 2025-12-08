#pragma once

class GUI {
private:
    void update();
    void startFrame();
    void endFrame();

    friend class Engine;

public:
    GUI();
    ~GUI();
    
    void drawTexture(unsigned int textureId);
};