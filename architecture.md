# Main architecture

```
src/
    engine/
        gui.cpp
        network.cpp
        render.cpp
        physics.cpp
        input.cpp
        audio.cpp
    client/
        main.cpp
        world.cpp
        shot.cpp
        player.cpp
        client.cpp
        radar.cpp
    server/
        main.cpp
        api.cpp
        shot.cpp
        client.cpp
```

## Engine
Each file will do its repsective task, and provides functions to interface with the the engine.

## Client
`main.cpp` will be in charge of the main loop and initializing parts of the engine. 