#pragma once
#include <vector>
#include "gl_header.h"

typedef std::vector<GLuint> GLIndice;

struct GLVertex;
struct HalfEdge;
struct Vertex;
struct Face;

struct GLVertex
{
    glm::vec3 position;
    glm::vec3 normal;

    GLVertex(glm::vec3 vpos, glm::vec3 vnorm = {0.f, 0.f, 0.f}) : position(vpos), normal(vnorm) {}
};

struct HalfEdge
{
    HalfEdge* twin = nullptr;     //twin half-edge
    HalfEdge* prev = nullptr;     //last half-edge in face
    HalfEdge* succ = nullptr;     //next half-edge in face
    Vertex* origin = nullptr;     //vertex which half-edge begins
    Face* incFace = nullptr;      //face which half-edge belongs

    bool is_boundary = false;
    // Constructor
};

struct Vertex
{
    int id;
    glm::vec3 position;
    HalfEdge* incEdge = nullptr;      //first half-edge from vertex
    glm::mat4 Q = glm::mat4(0.f);
    bool is_boundary = false;
};

struct Face
{
    HalfEdge* incEdge = nullptr;
    glm::mat4 Kp = glm::mat4(0.f);
    glm::vec4 norm = glm::vec4(0.f);
    bool operator == (const Face& face) const
    {
        return incEdge == face.incEdge;
    }
};
