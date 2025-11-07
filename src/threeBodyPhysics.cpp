#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm/glm.hpp>
#include <string>
using namespace glm;
using namespace std;
#define G 6.67430e-11
#include <fstream>   
#include <sstream>   
#include <string>
#include <cmath>

// structure for a particle in the three body system
typedef struct{
    dvec3 position;
    dvec3 velocity;
    float mass;

    float radius;
    string name;
    dvec3 color;
} sunBody;

// the gravitaitonal force of body2 on body1 --> F12
dvec3 gravForce(sunBody body1, sunBody body2) {
    dvec3 r = body2.position - body1.position;
    return r * (G * body2.mass) / pow(length(r), 3);
}

// computes new system for RK4
void system(sunBody body1, sunBody body2, sunBody body3, dvec3 *system) {
    dvec3 a1 = gravForce(body1, body2) + gravForce(body1, body3);
    dvec3 a2 = gravForce(body2, body1) + gravForce(body2, body3);
    dvec3 a3 = gravForce(body3, body1) + gravForce(body3, body2);

    system[0] = body1.velocity;
    system[1] = a1;
    system[2] = body2.velocity;
    system[3] = a2;
    system[4] = body3.velocity;
    system[5] = a3;
}

https://trinket.io/glowscript/038b9f8cfc