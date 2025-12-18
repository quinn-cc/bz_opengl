#pragma once

class GUI {
    friend class Engine;

private:
    void update();

    GUI();
    ~GUI();

public:
    void drawTexture(unsigned int textureId);
};