# BZFlag v3
I think BZFlag is on version 2, so this is version 3. Whatever.

## Overview
Here are the list of libaries currently used for each task:
* Raylib: Currently used for all rendering. We will probably switch to threepp very soon. Raylib does not support shadows which kind of sucks.
* ENet: Awesome UDP-based library for networking.
* CXXOpts: Easy library for taking in command-line arguments. Much better than C++'s built in one.
* GLM: Used for geometry calculations like vectors, quaternions, etc.. It is a rendering-agnostic way of storing position, rotation, etc.
* SPDLog: Simple lightweight liibrary for logging to console. I welcome any debugging to be left in. That being said, do not just use printf. See the SPDLog instead.

## To-do
Please try to follow style where it is obvious. I use `CapitalCamelCase` for all public methods, and `lowerCaseCamelCase` for private methods. All fields and local variables are `lowerCaseCamelCase`, and all globals are `g_globalVariable`, though I recommend against using globals. Try to use the singleton class scheme. There are some methods where I mix snake case and camel case. I know you hate this, but I do it where it makes sense to group methods by type. For instance `MsgRecv_Location` is one method, relating to receiving messages, but noting that it is the one regarding location.

### Switching rendering to threepp
This may be a bit of a learning curve, but if you're into graphics then take it. Threepp is really cool and supports lighting and model rendering in a scene environment. However, it does not support any GUI stuff, so you'll probably need to integrate with Dear ImGui. It's a really good renderer-agnostic library that can tap right into threepp, or another if we decide to change the rendering engine later.

Skills:
* Rendering. Probably only need to change `renderer.cpp`.

### Creating the chat
If you want a small hack at the networking and a bit of rendering, grab this task. You will have to use the gui. Right now we are using raylib, however if that is changed, please use the GUI interface like Dear ImGui. If this task is done before the switch to threepp, we'll probably just have to switch to Dear ImGui later. Shouldn't be a big deal, but that being said, it might be better to just create a new file like `gui.cpp` that interfaces with with Dear ImGui, which in turn taps into raybli.

Skills:
* GUI/Rendering. Again, should be separate, though you could just do this in Raylib but it will probably have to be changed. So if you do change `renderer.cpp`, we might just have to rip it all out later. Probably best to create a new script `gui.cpp` and add all the gui stuff there, and call it from `main.cpp.` Please follow all the existing style by making it a singleton class. See the other files in client for how to do that.
* Networking. Of course will have to change `networker.cpp` on both the client and the server to accommodate the messages.

### Creating the radar
This should probably be done AFTER the threepp conversion as it heavily relies on the graphics engine. You'll need to change the `renderer.cpp` to include a radar on the screen. You'll probably have to play around with the threepp wireframe mode as well as writing a custom shader. The radar will need to reflect buildings and the players. Buildings should be in wireframe, players as dots. In the original BZFlag, the color of the building changes depending on how close the player's y is to that level of the building. If you don't believe me, go play the game. If you think there is a better way of displaying that then have at it.

Skills:
* Rendering. WAIT TILL AFTER threepp is used. Will have to create a radar on the screen. Probably set up another camera in the screen that captures the scene and writes it to an image. Make another function for this please in `renderer.cpp`. Have some style.
* Shaders. Can get a prototype working without fancy shaders, but you will need to write a shader that renders the buildings different depending on how close the player's y is to the pixel. Again, this is last. Also ask ChatGPT how to do it. Give it a good prompt and it will get it for you. Then see how you can link the shader in threepp.

### Intro GUI screen
If you're gonna do so much GUI, just tie in Dear ImGui at this point. Make a `gui.cpp` file and add in stuff like "Join game", "enter username here", "enter port", blah blah blah. All the good stuff you do before joining the game. But here's the kicker. Don't just make it blot out the background or happen BEFORE a scene is rendered. What if the user wants to access that GUI stuff while they are currently in a server? It needs to be modular. Also add stuff for being able to change key bindings

Skills:
* GUI: Make a `gui.cpp` and do all the above.
* Input: If you're gonna do stuff like make the screen where you can add keybindings, then edit `input.cpp` to capture this stuff.

### Server list
We can connect to servers by typing in an IP and port but how will we ever know the IP and port??? This is where a server list comes in. We need a third binary that runs all the time on a server with a known IP and port. I can put that server list program on an AWS with a hardcoded IP and port, and the regular client binary can contact that for the server list. Then, when someone starts a new server you can contact the server binary and ask for new servers. If you think doing a HTTP post and get is better, have at it. Do whatever that makes this work. Probably you'll need to work with the last task to integrate into the GUI also.

This is probably a larger task that is harder to integrate since you'll have to create your own binary to do so. Proceed with caution.

Skills:
* Networking: Need a way to ping the server list, so you'll need to edit `networker.cpp` for sure.
* Creating new binary (CMAKE): Make the server list binary
* GUI: Work with previous task to help display.

### Implementing physics via Bullet, jumping
This is a biggie, but probably not too bad. Bullet looks like the way to go. It's lightweight, fast, and supports simple collision, exactly what we need. You'll need to assign a rigidbody to the player and have a way to see if they're grounded or not. Then you'll need to implement gravity and make them jump. Then you'll need to assign a collider to the floor. Don't worry about doing any other objects in the scene yet, that'll be later. Just gravity and a floor. But it needs to be legit floor collision, not just "stop when we hit y=0". Maybe you could test by adding cubes to the scene, whatever.

Skills:
* Physics: You'll need to read up about bullet and implement the `physics.cpp` file in the client, and call it from main. Link it to the player position, drive it when needed for gravity.
* Input: For jumping, you'll need to add the input for jumping, but this is simple.
* Player: Probably need to add some flags like "isGrounded", but not sure yet. You have at it. Only add what is necessary.

## License
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy.

Wouldn't think I would need a license for a BZFlag recreation, but yes, people have actually asked me what the licenses are on my mods.