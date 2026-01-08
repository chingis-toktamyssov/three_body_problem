#pragma once
#include <glm/glm.hpp>

using namespace glm;

// Simple body struct
struct sunBody {
    dvec3 position;
    dvec3 velocity;
    double mass;
};

// --- Function declarations ---
// NOTE: pass by VALUE, exactly like your original
dvec3 gravForce(sunBody body1, sunBody body2);

void accVelo(
    sunBody& body1,
    sunBody& body2,
    sunBody& body3,
    dvec3* system
);

void rk4(
    sunBody& body1,
    sunBody& body2,
    sunBody& body3,
    double dt
);
