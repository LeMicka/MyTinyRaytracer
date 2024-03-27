#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "mygeometry.h"

struct LightSource
{
    LightSource(const Vec3f &p, const float &i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};

struct Material
{
    Material(const float &r, const Vec4f &a, const Vec3f &color, const float &spec) : refractiveIndex(r), albedo(a), diffuseColor(color), specularExponent(spec) {}
    Material() : refractiveIndex(1), albedo(1, 0, 0, 0), diffuseColor(), specularExponent() {}
    float refractiveIndex;
    Vec4f albedo;
    Vec3f diffuseColor;
    float specularExponent;
};

struct Sphere
{
    Vec3f center;
    float radius;
    Material material;

    Sphere(const Vec3f &c, const float &r, const Material &m) : center(c), radius(r), material(m) {}

    bool rayIntersect(const Vec3f &orig, const Vec3f &dir, float &t0) const 
    {
        Vec3f L = center - orig;                                            // L is the vector from the origin to the center of the circle
        float tca = L.dotProduct(dir);                                      // dot product of L and dir : projection of L on dir
        float d2 = L.dotProduct(L) - tca * tca;                             // pythagore to calculate dsquare(distance sphere center, closest ray point)
        if (d2 > radius * radius)                                           // if ray passes out of the circle radius return false
            return (false);
        float thc = sqrt(radius * radius - d2);                             // calculate distance between closest ray point to center and t0
        t0 = tca - thc;                                                     // calculate t0, the first point of intersection ray-sphere
        float t1 = tca + thc;                                               // calculate t1, the second point of intersection ray-sphere
        if (t0 < 0)
            t0 = t1;
        if (t0 < 0)
            return (false);
        return (true);
    }
};

Vec3f reflectionVect(const Vec3f &vecLightDir, const Vec3f &hitPointNormal)
{
    return (2 * hitPointNormal * hitPointNormal.dotProduct(-vecLightDir) + vecLightDir);
}

Vec3f refractionVector(const Vec3f &vecLightDir, const Vec3f &hitPointNormal, const float &refractiveIndex)
{
    float cosLightNormal = - std::max(-1.f, std::min(1.f, vecLightDir.dotProduct(hitPointNormal)));         // if 0, lightVec perpendicular to normal, if 1 paralell and if -1 paralell but on the oposite side
    float etai = 1;                                                                                         // n1: refractive index of the incident substance, here air or void 
    float etat = refractiveIndex;                                                                           // n2: refractive index of the transmitting substance, hee glass
    Vec3f n = hitPointNormal;

    if (cosLightNormal < 0)                                                                                 // if the ray is inside the object swap indices and invert normal 
    {
        cosLightNormal = -cosLightNormal;
        n = -hitPointNormal;
        std::swap(etai, etat);
    }
    float refractiveIndexesRatio = etai / etat;
    float k = 1 - refractiveIndexesRatio * refractiveIndexesRatio * (1 - cosLightNormal * cosLightNormal);  // if k<0 that means no refraction and total reflection 

    if (k < 0)                                                                                              // if k is smaller than 0, there is no vector to refract but we return a small vector aniway. 
        return (Vec3f(1, 0, 0));
    return (vecLightDir*refractiveIndexesRatio + n * (refractiveIndexesRatio * cosLightNormal - sqrt(k)));  // complete snell's law formula to compute the refracted ray
}

bool sceneIntersection (const Vec3f &origin, const Vec3f &direction, const std::vector<Sphere> &spheres, Vec3f &hitPoint, Vec3f &normal, Material &material)
{
    float sphereDist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < spheres.size(); i++)
    {
        float distanceIntersection;
        if (spheres[i].rayIntersect(origin, direction, distanceIntersection) && distanceIntersection < sphereDist) //to make sure the closest sphere is rendered
        {
            sphereDist = distanceIntersection;
            hitPoint = origin + direction * distanceIntersection;
            normal = (hitPoint - spheres[i].center).normalize();
            material = spheres[i].material;
        }
    }

    float checkerBoardDist = std::numeric_limits<float>::max();
    if(fabs(direction.y) > 1e-3)                                    // checks if y of direction vector is not 0, to prevent a division by 0
    {
        float distPHit = -(origin.y + 4) / direction.y;             // calculate the distance origin - point hitting the plane
        Vec3f pHitPosition = origin + direction * distPHit;         // position of the point hiting the plane
        if (distPHit > 0 && distPHit < sphereDist && fabs(pHitPosition.x) < 10 && pHitPosition.z < -10 && pHitPosition.z > -30)
        {
            checkerBoardDist = distPHit;
            hitPoint = pHitPosition;
            normal = Vec3f(0, 1, 0);
            material.diffuseColor = (int(0.5 * hitPoint.x + 1000) + int(0.5 * hitPoint.z)) & 1 ? Vec3f(.3, .3, .3) : Vec3f(.3, .2, .1);
        }
    }

    return (std::min(sphereDist, checkerBoardDist) < 1000);                                                                                    // render only the spheres that are less than 1000 away
}



Vec3f castRay (const Vec3f &origin, const Vec3f &direction, const std::vector<Sphere> &spheres, const std::vector<LightSource> &lights, size_t depth=0)
{
    Vec3f hitPoint;
    Vec3f normal;
    Material material;
    float diffuseLightIntensity = 0;
    float specularLightIntensity = 0;

    if (depth > 4 || !sceneIntersection(origin, direction, spheres, hitPoint, normal, material))
    {
        return (Vec3f(0.2, 0.7, 0.8));
    }

    Vec3f reflectionDirection = reflectionVect(direction, normal).normalize();
    Vec3f reflectionOrigin = hitPoint + normal*1e-3;
    Vec3f reflectionColor = castRay(reflectionOrigin, reflectionDirection, spheres, lights, depth + 1);

    Vec3f refractionDirection = refractionVector(direction, normal, material.refractiveIndex).normalize();
    Vec3f refractionOrigin = refractionDirection.dotProduct(normal) < 0 ? hitPoint - normal*1.e-3 : hitPoint + normal*1e-3;
    Vec3f refractionColor = castRay(refractionOrigin, refractionDirection, spheres, lights, depth + 1);

    for (size_t i=0; i < lights.size(); i++)
    {
        Vec3f lightDir = (lights[i].position - hitPoint).normalize();
        float hitpointLightDistance = (lights[i].position - hitPoint).length();
        Vec3f shadowOrigin = hitPoint + normal*1e-3;                                        //Modified because i don't see why we would ever need to subtract Normal*1e-3 as adding it will always make sure that the origin is outside of the surface and in the direction of the normal.
        Vec3f shadowHitPoint;
        Vec3f shadowNormal;
        Material tmpMaterial;
        if (sceneIntersection(shadowOrigin, lightDir, spheres, shadowHitPoint, shadowNormal, tmpMaterial) && (shadowHitPoint-shadowOrigin).length() < hitpointLightDistance)
            continue;
        diffuseLightIntensity += lights[i].intensity * std::max(0.f, lightDir.dotProduct(normal));
        specularLightIntensity += powf(std::max(0.f, reflectionVect(lightDir, normal).dotProduct(direction)), material.specularExponent) * lights[i].intensity;
    }
    return (material.diffuseColor * diffuseLightIntensity * material.albedo[0] + Vec3f(1., 1., 1.)*specularLightIntensity * material.albedo[1] + reflectionColor * material.albedo[2] + 
            refractionColor * material.albedo[3]);
}

void render(const std::vector<Sphere> &spheres, const std::vector<LightSource> &lights)
{
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

    std::ofstream ofs; // save the framebuffer to file
    ofs.open("./out.ppm");
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height*width; ++i) {
        Vec3f &c = framebuffer[i];
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max > 1)
            c = c*(1./max);
        for (size_t j = 0; j<3; j++) {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();
}


int main()
{
    Material      ivory(1.0, Vec4f(0.6,  0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3),   50.);
    Material      glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8),  125.);
    Material      red_rubber(1.0, Vec4f(0.9,  0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1),   10.);
    Material      mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);
    std::vector<Sphere> spheres;
    std::vector<LightSource> lights;

    //std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3,    0,   -16), 2,      ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2,      glass));
    spheres.push_back(Sphere(Vec3f( 1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(Vec3f( 7,    5,   -18), 4,     mirror));
    //std::vector<Light>  lights;
    lights.push_back(LightSource(Vec3f(-20, 20,  20), 1.5));
    lights.push_back(LightSource(Vec3f( 30, 50, -25), 1.8));
    lights.push_back(LightSource(Vec3f( 30, 20,  30), 1.7));

    render(spheres, lights);
    return (0);
}