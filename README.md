# Laky's Awsome Raycaster
<br>

Have you ever looked at retro games like id Tech's Wolfenstein 3D and wondered, *"Wow! That's so cool! I wonder how they did that"*? Well, I have! 
Thankfully, the magical place we call the internet is full of useful guides, tutorials and other material about the tech behind Wolf3D and similar games - Raycasting. <br>
So I decided to follow them, and with 50% guides and 50% banging my head against the monitor my raycaster was born!

What can this little demo raycaster do:
- Minimap
- Textured walls
- Textured ceilings
- Textured floors
- Sky rendering
- Loading of any size and resolution textures via stb_image
- Supports different resolutions (currently turned off, but it would be only a couple of lines to turn it on!)
- Relatively fast

Used libraries:
- GLFW
- GLAD
- stb_image

You can also play a (bit outdated) build of the game online, thanks to Emscripten: https://lbence-szixi.github.io/projects/raycaster/

<br>

# How to build
Just set up GLFW and GLAD in any development environment you like. Then put stb_image.h in the <code>src</code> folder, and build out the project using the Makefile! <br>
**WARNING: I only tested the code on Windows. I can not guarantee that it will build on Linux or Mac OS.**
