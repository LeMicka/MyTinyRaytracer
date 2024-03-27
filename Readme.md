# My first step into the world of graphics programming with a raytracer.

This repository is my version of this project: [tiniraytracer](https://github.com/ssloy/tinyraytracer/wiki). I would like to thank [ssloy](https://github.com/ssloy) for the amazing tutorial, this project was very challenging even with the wiki explanations so i will try to give a small explanation of my code too.

First, check [the wiki](https://github.com/ssloy/tinyraytracer/wiki) of the original version of this project with the complete step by step.
 
# Mygeometry.h

You'll see my geometry file is quite different, the one used by ssloy was very confusing to me so I looked for another way of doing the vector classes and i found the one of [scratchapixel](https://www.scratchapixel.com/index.html), also I only declared vector 3, 3, 4 classes as we don't need matrices in this project. I had to modify the Vec2 class to be able to call it's x and y elements like v[0] = x and V[1] = y and not just v.x and v.y, this was already in the Vec3 class.

# The Code

```c++
const int width = 1024;
const int height = 768;
const int fov = M_PI/2.;                                                                // pi/2 : 90º
std::vector<Vec3f> framebuffer(width * height);
float x;
float y;

#pragma omp parallel for
for (size_t j = 0; j < height; j++)
{
    for (size_t i = 0; i < width; i++)
    {
        float imageAspectRatio = width / (float)height;                                        //assuming width > height
        x = (2 * (i + 0.5) / (float)width - 1) * tan(fov/2.) * imageAspectRatio;               //tan(fov/2) just gives us the length from the center of the screen to the edge. and here it is one because fov = 90º, but changing that will give us more or less field of view 
        y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov/2.);
        Vec3f dir = Vec3f(x, y, -1).normalize();
        framebuffer[i+j*width] = castRay(Vec3f(0,0,0), dir, spheres, lights);
    }
}
```


## compilation
```sh
git clone https://github.com/ssloy/tinyraytracer.git
cd tinyraytracer
mkdir build
cd build
cmake ..
make
```

