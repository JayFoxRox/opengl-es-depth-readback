OpenGL ES does not support glReadPixels to read back the depth values.
As I require this for one of my projects, I started looking for workarounds.

While this workaround is hacky, it should work for a handful of individual pixels on my target platform.

# Building and running

```
git clone https://github.com/JayFoxRox/opengl-es-depth-readback.git
cd opengl-es-depth-readback
mkdir build
cd build
cmake ..
make
./cube
```
