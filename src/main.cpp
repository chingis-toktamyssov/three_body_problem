#include "threeBodyPhysics.h"

// -------------------- OpenGL --------------------
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// -------------------- GLM --------------------
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

// -------------------- STL --------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// ============================================================
//                         CONFIG
// ============================================================
#define TRAIL_LENGTH 2000
#define PHYSICS_STEPS_PER_FRAME 10

// ============================================================
//                     TRAIL STORAGE
// ============================================================
vector<vec3> trail1;
vector<vec3> trail2;
vector<vec3> trail3;

// ============================================================
//                     TRAIL HELPERS
// ============================================================
void updateTrail(vector<vec3>& trail, const vec3& pos) {
    trail.push_back(pos);
    if (trail.size() > TRAIL_LENGTH) trail.erase(trail.begin());
}

void drawTrail(const vector<vec3>& trail, GLuint lineVAO, GLuint lineVBO,
               GLuint shader, vec3 color, const mat4& proj) {
    if (trail.empty()) return;
    glUseProgram(shader);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, trail.size() * sizeof(vec3), trail.data());
    glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));
    glUniformMatrix4fv(glGetUniformLocation(shader, "uProjection"), 1, GL_FALSE, value_ptr(proj));
    glDrawArrays(GL_LINE_STRIP, 0, trail.size());
}

// ============================================================
//                    SHADER UTILITIES
// ============================================================
string loadShader(const string& path) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Failed to load shader: " << path << endl;
        return "";
    }
    stringstream ss; ss << file.rdbuf();
    return ss.str();
}

GLuint compile(GLenum type, const string& src) {
    GLuint shader = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(shader, 1, &c, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512]; glGetShaderInfoLog(shader, 512, nullptr, log);
        cerr << "Shader compile error:\n" << log << endl;
    }
    return shader;
}

GLuint createProgram(const string& vertPath, const string& fragPath) {
    GLuint vs = compile(GL_VERTEX_SHADER, loadShader(vertPath));
    GLuint fs = compile(GL_FRAGMENT_SHADER, loadShader(fragPath));
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

// ============================================================
//                          MAIN
// ============================================================
int main() {

    // ---------------- GLFW INIT ----------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 800, "3 Body Problem", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGL();

    glPointSize(16.0f);

    // ---------------- SHADERS ----------------
    GLuint bodyShader = createProgram("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    GLuint lineShader = createProgram("../shaders/line.vert", "../shaders/line.frag");

    // ---------------- POINT VAO/VBO ----------------
    float points[9] = {0,0,0, 0,0,0, 0,0,0};
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // ---------------- LINE VAO/VBO ----------------
    GLuint lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, TRAIL_LENGTH*sizeof(vec3), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
    glEnableVertexAttribArray(0);

    // ---------------- PHYSICS SETUP ----------------
    sunBody b1, b2, b3;
    double dt = 0.0005;

    b1.position = dvec3(1, -0.24308753, 0);
    b1.velocity = dvec3(0.93240737/2, 0.8643146/2, 0); b1.mass = 1;

    b2.position = dvec3(-0.97000436, 0.24308753, 0);
    b2.velocity = dvec3(0.93240737/2, 0.8643146/2, 0); b2.mass = 1;

    b3.position = dvec3(0,0,0);
    b3.velocity = dvec3(0.93240737, -0.8643146,0); b3.mass = 1;

    vec3 color1(1.0f, 0.2f, 0.2f);
    vec3 color2(0.2f, 0.8f, 1.0f);
    vec3 color3(1.0f, 1.0f, 0.2f);

    GLint projLoc  = glGetUniformLocation(bodyShader, "projection");
    GLint viewLoc  = glGetUniformLocation(bodyShader, "view");
    GLint colorLoc = glGetUniformLocation(bodyShader, "bodyColor");

    // ---------------- CAMERA STATE ----------------
    vec2 cameraCenter = vec2(0.0f);
    float cameraZoom = 1.0f;
    const float PAN_SPEED  = 1.5f;
    const float ZOOM_SPEED = 1.2f;

    // ---------------- MAIN LOOP ----------------
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);

        // -------- CAMERA CONTROLS --------
        float dtCam = 0.016f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraCenter.y += PAN_SPEED * dtCam * cameraZoom;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraCenter.y -= PAN_SPEED * dtCam * cameraZoom;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraCenter.x -= PAN_SPEED * dtCam * cameraZoom;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraCenter.x += PAN_SPEED * dtCam * cameraZoom;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cameraZoom *= (1.0f + ZOOM_SPEED*dtCam);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cameraZoom *= (1.0f - ZOOM_SPEED*dtCam);
        cameraZoom = glm::clamp(cameraZoom, 0.05f, 50.0f);

        // -------- DYNAMIC PROJECTION --------
        float aspect = 1.0f; // square window
        float viewSize = 2.0f * cameraZoom;
        mat4 proj = ortho(cameraCenter.x - viewSize*aspect, cameraCenter.x + viewSize*aspect,
                          cameraCenter.y - viewSize, cameraCenter.y + viewSize,
                          -1.0f, 1.0f);
        mat4 view = mat4(1.0f);

        // -------- PHYSICS --------
        for (int i = 0; i < PHYSICS_STEPS_PER_FRAME; i++) rk4(b1,b2,b3,dt);

        float updated[9] = {
            (float)b1.position.x,(float)b1.position.y,0,
            (float)b2.position.x,(float)b2.position.y,0,
            (float)b3.position.x,(float)b3.position.y,0
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(updated), updated);

        updateTrail(trail1, (vec3)b1.position);
        updateTrail(trail2, (vec3)b2.position);
        updateTrail(trail3, (vec3)b3.position);

        // -------- DRAW BODIES --------
        glUseProgram(bodyShader);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(proj));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
        glBindVertexArray(VAO);

        glUniform3fv(colorLoc,1,value_ptr(color1)); glDrawArrays(GL_POINTS,0,1);
        glUniform3fv(colorLoc,1,value_ptr(color2)); glDrawArrays(GL_POINTS,1,1);
        glUniform3fv(colorLoc,1,value_ptr(color3)); glDrawArrays(GL_POINTS,2,1);

        // -------- DRAW TRAILS --------
        drawTrail(trail1,lineVAO,lineVBO,lineShader,color1,proj);
        drawTrail(trail2,lineVAO,lineVBO,lineShader,color2,proj);
        drawTrail(trail3,lineVAO,lineVBO,lineShader,color3,proj);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
