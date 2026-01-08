#version 330 core

out vec4 FragColor;
uniform vec3 bodyColor;

void main()
{
    // gl_PointCoord is in range [0,1] x [0,1]
    vec2 coord = gl_PointCoord - vec2(0.5);

    // distance from center of point
    float dist = length(coord);

    // discard fragments outside a circle
    if (dist > 0.5)
        discard;

    FragColor = vec4(bodyColor, 1.0);
}
