#pragma once
#include "gl_header.h"
#include "Shader.h"
#include "ObjLoader.h"
#include "primitives.h"

#include <unordered_map>
#include <vector>
#include <string>

struct EdgeKey
{
    int v1, v2;
    EdgeKey(int vd1 = -1, int vd2 = -1) : v1(vd1), v2(vd2) {}

    bool operator==(const EdgeKey& key) const
    {
        return v1 == key.v1 && v2 == key.v2;
    }
};
struct EdgeKeyHashFuc
{
    std::size_t operator()(const EdgeKey& key) const
    {
        return std::hash<int>()(key.v1) + std::hash<int>()(key.v2);
    }
};


// DCEL impletement of 3D mesh
class Mesh
{
public:
    Mesh();
    ~Mesh();
    // Mesh(const Mesh& mesh);

    // DCEL construct methods
    Face* createFace(Vertex* vertexs[3]);
    HalfEdge* createHalfEdge(Vertex* v1, Vertex* v2);
    Vertex* createVertex(const glm::vec3& vpos);

    //void update_vertex(Vertex* v);
    void init_face(Face* face);
    void update_Q();
    void flag_boundary();
    bool delete_face(Face* face);
    bool delete_halfedge(HalfEdge* he);
    bool delete_vertex(Vertex* v);

    bool has(Vertex* v);
    bool has(HalfEdge* he);
    bool has(Face* face);

    std::vector<HalfEdge*> get_incEdge(Vertex* v);
    std::vector<Vertex*> get_neighbor_vertex(Vertex* v);

    // load .obj file
    bool load_file(std::string obj_path);

    void auto_normal();

    void draw(const Shader& shader);

    glm::mat4 GetModelMatrix();

    // clean out data
    void clear();

    void setupGL();
    void updateGL();

    // DCEL
    std::vector<HalfEdge*> m_edges;
    std::vector<Face*> m_faces;
    std::vector<Vertex*> m_vertexes;
    
private:
    // initialize OpenGL objects and buffer
    

    glm::vec3 m_mesh_pos;
    glm::vec3 m_mesh_rot;

    GLuint VAO, VBO, EBO_TRI;

    // RAW
    std::vector<GLVertex> m_GLvertexes;
    GLIndice m_GLindices;

    std::unordered_map<EdgeKey, HalfEdge*, EdgeKeyHashFuc> m_hashmap_edge;

    int m_edges_cnt = 0;
    int m_faces_cnt = 0;
    int m_vertexes_cnt = 0;

    ObjLoader* objloader;
};