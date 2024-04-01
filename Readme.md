# My first step into the world of graphics programming with a raytracer.

This repository is my version of this project: [tiniraytracer](https://github.com/ssloy/tinyraytracer/wiki). I would like to thank [ssloy](https://github.com/ssloy) for the amazing tutorial, this project was very challenging even with the wiki explanations so I will try to give a small explanation of my code too.

First, check [the wiki](https://github.com/ssloy/tinyraytracer/wiki) of the original version of this project with the complete step by step.
 
## Explanation of the program
 
This is a raytracer, raytracing consists of sending a ray from the camera into the scene and bouncing them on surfaces towards sources of lights to aproximate color of each pixels. 

## Mygeometry.h

You'll see my geometry file is quite different, the one used by ssloy was very confusing to me so I looked for another way of doing the vector classes and I found the one of [scratchapixel](https://www.scratchapixel.com/index.html), also I only declared vector 3, 3, 4 classes as we don't need matrices in this project. I had to modify the Vec2 class to be able to call it's x and y elements like v[0] = x and V[1] = y and not just v.x and v.y, this was already in the Vec3 class.

## The Code

As this is a raycaster, we need to generate rays for each pixel in the frame, and each ray is generated from the camera to the scene.
For this, we need to calculate the direction of each ray, cast it and store the color in a vector.

```c++
const int width = 1024;
const int height = 768;
const int fov = M_PI/2.;
std::vector<Vec3f> framebuffer(width * height);
float x;
float y;

#pragma omp parallel for
for (size_t j = 0; j < height; j++)
{
    for (size_t i = 0; i < width; i++)
    {
        float imageAspectRatio = width / (float)height;
        x = (2 * (i + 0.5) / (float)width - 1) * tan(fov/2.) * imageAspectRatio;
        y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov/2.);
        Vec3f dir = Vec3f(x, y, -1).normalize();
        framebuffer[i+j*width] = castRay(Vec3f(0,0,0), dir, spheres, lights);
    }
}
```
fov: the angle of the field of vue, here it is PI/2 radians so 90º.
dir: the direction vector of the ray it is normalised because it is a direction.

How to calculate the x and y of the direction vector: 

Imagine the frame as a 2D plane with the origin at it's center and the limits of the frame on on the x and y axis are -1 and 1 [see](https://www.scratchapixel.com/images/ray-tracing-camera/cambasic.png?). 

```c++
        x = (2 * (i + 0.5) / (float)width - 1) * tan(fov/2.) * imageAspectRatio;
        y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov/2.);
```
-First, we add 0.5 to i and j to make sure that we cast the ray through the middle of each pixel.

-Then we remap the value in range [0, 1] by dividing it by the screen width or height in pixels.

-Now we want these values to be in range [-1, 1] (see the image of the frame above) to do so, we multiply by 2 so we are in range [0, 2] and then substract one to get the range [-1, 1].

-Something is wrong with the y coordinate here because if j = 0 here we will get y = -1 but we need it to be 1 as we will start with the top left corner whitch is the position [-1, 1] so we put the negative sign to inverse it.

-The image we get here is a rectangle because the width is bigger than the height. So we actually need to multiply the x (width) coordinate by the image aspect ratio. So for example the x coordinate will be in range [-1.4, 1.4] instead of [-1, 1].

-And finally we need to account the field of view. In our case the fov is 90º, so we have tan(fov/2) = 1, this is geometry look at [this picture](https://www.scratchapixel.com/images/ray-tracing-camera/camprofile.png?).

-We know that in a square triangle tan(alpha) = opposite/adjacent, and we know that the adjacent (length between camera and frame is 1) so opposite = tan(alpha) * 1. In our case that means tan(fov/2) is the length between the center of the frame 
and the edge. We can observe that the value of the fov will change that length, so an angle bigger than 90 will give us a value bigger than 1, and a smaller one a value smaller than 1.

The castRay function:

This Function casts the ray and returns the color of the pixel.
If the ray doesn't intersect with any object, it returns a generic backgroud color of our choice.
And if it does intersect, the function will return the color calculated as explained below.
The color is a mix of different things: the diffuse color, the spacular color, the reflection and the refraction.

The diffuse lighting color:
We need the lightdDirection vector that we can get doint light position - hitpoint.
With this we calculate the diffuse lighting of the light source at this point multiplying the intensity of the light source to the dotproduct of the lightDirection and the normal.

The specular lighting color:
See the [phong reflection model](https://en.wikipedia.org/wiki/Phong_reflection_model) for the explanation. 

The shadows:
What we do here is we tke the hitpoint of the ray to the object and and cast a ray from that point to the light to see if it intersects with any object before reaching the light source, if it does, we skip that light source.
The origin of the shadow ray is moved with a small offset in the direction of the normal because as the hitpoint is on the surface of the object, any ray from this point will intersect the object itself.   

The reflection:

```c++
    Vec3f reflectionDirection = reflectionVect(direction, normal).normalize();
    Vec3f reflectionOrigin = hitPoint + normal*1e-3;
    Vec3f reflectionColor = castRay(reflectionOrigin, reflectionDirection, spheres, lights, depth + 1);
```
As for the shadow we will cast a ray from the hitpoint in the direction of the reflection that we get from the reflection vector formula explained in the Phong reflection model.
Here we will cast a ray up to a depth of 4 whitch means that we will render up to 4 bounces for each ray.

The refraction:

```c++
    Vec3f refractionDirection = refractionVector(direction, normal, material.refractiveIndex).normalize();
    Vec3f refractionOrigin = refractionDirection.dotProduct(normal) < 0 ? hitPoint - normal*1.e-3 : hitPoint + normal*1e-3;
    Vec3f refractionColor = castRay(refractionOrigin, refractionDirection, spheres, lights, depth + 1);
```
For the refraction we first have to calculate the refraction direction, see [Snell's law](https://en.wikipedia.org/wiki/Snell%27s_law) especially the vector form part.
The origin is moved positively or negatively based on the dotproduct result as the hitpoint can be outside or inside of an object.

Creating the plane and the checkerboard patern:

```c++
    float checkerBoardDist = std::numeric_limits<float>::max();
    if(fabs(direction.y) > 1e-3)                                    
    {
        float distPHit = -(origin.y + 4) / direction.y;             
        Vec3f pHitPosition = origin + direction * distPHit;         
        if (distPHit > 0 && distPHit < sphereDist && fabs(pHitPosition.x) < 10 && pHitPosition.z < -10 && pHitPosition.z > -30)
        {
            checkerBoardDist = distPHit;
            hitPoint = pHitPosition;
            normal = Vec3f(0, 1, 0);
            material.diffuseColor = (int(0.5 * hitPoint.x + 1000) + int(0.5 * hitPoint.z)) & 1 ? Vec3f(.3, .3, .3) : Vec3f(.3, .2, .1);
        }
    }
```
First, we check if the y component is 0 to avoid a division by 0.

```c++
float distPHit = -(origin.y + 4) / direction.y;             
Vec3f pHitPosition = origin + direction * distPHit;         
```
The equation of the plane is y = -4, whitch means that the plane is on the XZ plane with y = -4.
We then calculate the distance from the ray origin to the plane whitch is actually the number of units from the ray origin until the ray reaches the plane's y.
With that we can get the position of that point.

We give the checkerboard a size of 20 by 20 , between -10 and 10 on x and between -10 and -30 on z.
If the hitpoint is between those coordinates, we can create the checkerboard patern:
```c++
material.diffuseColor = (int(0.5 * hitPoint.x + 1000) + int(0.5 * hitPoint.z)) & 1 ? Vec3f(.3, .3, .3) : Vec3f(.3, .2, .1);        
```
To do this, we use the formula int(hitPoint.x) + int(hitPoint.z) & 1, this will give either 0 or 1 depending on if the first part is even or odd. based on the result, we assign one color or the other.
In our case we scale by 0.5 the x and the z to get a patern of 10x10 squares instead of 20x20.
We then add 1000 to x, this comes from the main tutorial, it is not explain why we do it and i am not sure if it is needed as I have tried changing it to different values even negative ones with no change to be seen.
I left it just in case but I think you can remove it.

## compilation
```sh
git clone https://github.com/ssloy/tinyraytracer.git
cd tinyraytracer
mkdir build
cd build
cmake ..
make
```

## Links for more informations

[Ray-Sphere intersection](https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html) : This is the best explanation I have found for this.  

[Generating rays from camera](https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays.html)

Mathematics: [Scratchapixel's geometry](https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/points-vectors-and-normals.html) section as well as [3blue1brown's linear algebra](https://www.3blue1brown.com/topics/linear-algebra) section.

Reflection : https://en.wikipedia.org/wiki/Phong_reflection_model
