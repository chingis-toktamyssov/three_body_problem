#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
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
    dvec3 acceleration;
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
void accVelo(sunBody &body1, sunBody &body2, sunBody &body3, dvec3 *system) {
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

void rk4(sunBody &body1, sunBody &body2, sunBody &body3, double dt) {
    dvec3 k1[6], k2[6], k3[6], k4[6];

    // k1
    accVelo(body1, body2, body3, k1);

    sunBody t1, t2, t3;

    // k2
    t1 = body1;
    t1.position = body1.position + k1[0] * (dt * 0.5);
    t1.velocity = body1.velocity + k1[1] * (dt * 0.5);

    t2 = body2;
    t2.position = body2.position + k1[2] * (dt * 0.5);
    t2.velocity = body2.velocity + k1[3] * (dt * 0.5);

    t3 = body3;
    t3.position = body3.position + k1[4] * (dt * 0.5);
    t3.velocity = body3.velocity + k1[5] * (dt * 0.5);

    accVelo(t1, t2, t3, k2);

    // k3
    t1 = body1;
    t1.position = body1.position + k2[0] * (dt * 0.5);
    t1.velocity = body1.velocity + k2[1] * (dt * 0.5);

    t2 = body2;
    t2.position = body2.position + k2[2] * (dt * 0.5);
    t2.velocity = body2.velocity + k2[3] * (dt * 0.5);

    t3 = body3;
    t3.position = body3.position + k2[4] * (dt * 0.5);
    t3.velocity = body3.velocity + k2[5] * (dt * 0.5);

    accVelo(t1, t2, t3, k3);

    // k4
    t1 = body1;
    t1.position = body1.position + k3[0] * (dt);
    t1.velocity = body1.velocity + k3[1] * (dt);

    t2 = body2;
    t2.position = body2.position + k3[2] * (dt);
    t2.velocity = body2.velocity + k3[3] * (dt);

    t3 = body3;
    t3.position = body3.position + k3[4] * (dt);
    t3.velocity = body3.velocity + k3[5] * (dt);

    accVelo(t1, t2, t3, k4);

    // final RK4 combine step
    body1.position += (dt/6.0) * (k1[0] + 2.0*k2[0] + 2.0*k3[0] + k4[0]);
    body1.velocity += (dt/6.0) * (k1[1] + 2.0*k2[1] + 2.0*k3[1] + k4[1]);

    body2.position += (dt/6.0) * (k1[2] + 2.0*k2[2] + 2.0*k3[2] + k4[2]);
    body2.velocity += (dt/6.0) * (k1[3] + 2.0*k2[3] + 2.0*k3[3] + k4[3]);

    body3.position += (dt/6.0) * (k1[4] + 2.0*k2[4] + 2.0*k3[4] + k4[4]);
    body3.velocity += (dt/6.0) * (k1[5] + 2.0*k2[5] + 2.0*k3[5] + k4[5]);
}

// int main() {
//     double dt = 0.0005, time = 500000;
//     sunBody body1, body2, body3;

//     body1.position = dvec3(0.97000436, -0.24308753, 0);
//     body1.velocity = dvec3(0.93240737/2, 0.8643146/2, 0);
//     body2.position = dvec3(-0.97000436, 0.24308753, 0);
//     body2.velocity = dvec3(0.93240737/2, 0.8643146/2, 0);    
//     body3.position = dvec3(0, 0, 0);
//     body3.velocity = dvec3(0.93240737, -0.8643146, 0);

//     for (int i = 0; i < time; i++) {
//         rk4(body1, body2, body3, dt);
//     }
// }