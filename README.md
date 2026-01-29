## Noice Maze

Version of the 3D Maze Screensaver from Windows 95. The geometry becomes visible through movement in a binary noise field.

Click the image to watch it on Youtube.

[![Watch the video](https://img.youtube.com/vi/2ELJjn2ffys/0.jpg)](https://www.youtube.com/watch?v=2ELJjn2ffys)


### Build

```
git clone https://github.com/erikrl2/NoiceMaze
cd NoiceMaze
cmake --preset ninja
cmake --build build/ninja --config Release
```
Requires vcpkg installation.
Or use Visual Studio on Windows with integrated vcpkg. CMake `--preset vs` generates a VS solution file.


### Credit

This is a fork of [GLMaze](https://github.com/ultitech/GLMaze).
I hooked up my noise effect ([Noice](https://github.com/erikrl2/Noice)) to the renderer.
