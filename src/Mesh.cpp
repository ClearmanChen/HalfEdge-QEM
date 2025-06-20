#include "Mesh.h"

Mesh::Mesh()
{
    objloader = new ObjLoader();
    VAO = 0;
    VBO = 0;
    EBO_TRI = 0;
    m_mesh_pos = glm::vec3(0.0);
    m_mesh_rot = glm::vec3(0.0);
}

Mesh::~Mesh()
{
    ;
}

//Mesh::Mesh(const Mesh& mesh)
//{
//    objloader = new ObjLoader();
//    VAO = 0;
//    VBO = 0;
//    EBO_TRI = 0;
//}


Face* Mesh::createFace(Vertex* vertexs[3])
{
    if (vertexs[0] == nullptr || vertexs[1] == nullptr || vertexs[3] == nullptr)
    {
        std::cerr << "ERROR::MESH::FACE_CREATE_FAILED" << std::endl;
        return nullptr;
    }
    Face* face = new Face();
    HalfEdge* edges[3];
    for (int i = 0; i < 3; i++) 
    {
        edges[i] = createHalfEdge(vertexs[i], vertexs[(i + 1) % 3]);
        if (edges[i] == nullptr) 
        {
            std::cerr << "ERROR::MESH::FACE_CREATE_FAILED" << std::endl;
            return nullptr;
        }

    }
    for (int i = 0; i < 3; i++) 
    {
        edges[i]->succ = edges[(i + 1) % 3];
        edges[i]->prev = edges[(i + 2) % 3];
        edges[i]->incFace = face;
        m_edges.push_back(edges[i]);
    }

    face->incEdge = edges[0];
    //Calculate Kp
    m_faces.push_back(face);
    init_face(face);
    return face;
}

HalfEdge* Mesh::createHalfEdge(Vertex* v1, Vertex* v2)
{
    if (v1 == nullptr || v2 == nullptr)
    {
        return nullptr;
    }
    EdgeKey key(v1->id, v2->id);

    // half-edge already existed
    if (m_hashmap_edge.find(key) != m_hashmap_edge.end())
    {
        return m_hashmap_edge[key];
    }

    HalfEdge* new_edge = new HalfEdge();
    HalfEdge* new_edge_twin = new HalfEdge();

    new_edge->origin = v1;
    new_edge->twin = new_edge_twin;

    new_edge_twin->origin = v2;
    new_edge_twin->twin = new_edge;

    v1->incEdge = new_edge;
    v2->incEdge = new_edge_twin;
    m_hashmap_edge[EdgeKey(v1->id, v2->id)] = new_edge;
    m_hashmap_edge[EdgeKey(v2->id, v1->id)] = new_edge_twin;
    return new_edge;
}

Vertex* Mesh::createVertex(const glm::vec3& vpos)
{
    Vertex* new_vert = new Vertex();
    new_vert->id = m_vertexes_cnt++;
    new_vert->position = vpos;
    new_vert->Q = glm::mat4(0.f);
    m_vertexes.push_back(new_vert);

    return new_vert;
}
/*
void Mesh::update_vertex(Vertex* v)
{
    auto incEdges = get_incEdge(v);
    for (auto& edge : incEdges)
    {
        if (edge->incFace != nullptr)
        {
            auto face = edge->incFace;
            update_face(face);
        }
    }
}*/

void Mesh::init_face(Face* face)
{
    if (face == nullptr || !has(face->incEdge))
    {
        std::cerr << "ERROR::MESH::UPDATE_FACE::FACE_NOT_EXIST" << std::endl;
        return;
    }
    Vertex* v0 = face->incEdge->origin;
    Vertex* v1 = face->incEdge->succ->origin;
    Vertex* v2 = face->incEdge->prev->origin;
    if (v0 == v1 || v0 == v2 || v1 == v2)
    {
        std::cerr << "ERROR::MESH::UPDATE_FACE::FACE_INVALID" << std::endl;
        return;
    }
    //Calculate Kp
    auto vec1 = v1->position - v0->position;
    auto vec2 = v2->position - v0->position;
    glm::vec3 plane_norm = glm::cross(vec1, vec2);

    plane_norm = glm::normalize(plane_norm);
    float plane_d = -glm::dot(plane_norm, v0->position);
    glm::vec4 plane(plane_norm, plane_d);
    face->Kp = glm::outerProduct(plane, plane);
    v0->Q += face->Kp;
    v1->Q += face->Kp;
    v2->Q += face->Kp;
}

void Mesh::update_Q()
{
    for (auto& v : m_vertexes)
    {
        v->Q = glm::mat4(0.f);
        for (auto& e : get_incEdge(v))
        {
            v->Q += e->incFace->Kp;
        }
    }
}

void Mesh::flag_boundary()
{
    if (m_edges.size() == 0) return;
    for (auto& edge : m_edges) edge->is_boundary = false;
    for (auto& v : m_vertexes) v->is_boundary = false;
    for (auto& edge : m_edges)
    {
        if (edge->incFace == nullptr)
        {
            edge->twin->origin->is_boundary = true;
            edge->twin->succ->origin->is_boundary = true;
            edge->twin->is_boundary = true;
        }
    }
}

bool Mesh::delete_face(Face* face)
{
    if (face == nullptr)
    {
        std::cerr << "ERROR::MESH::DELETE_FACE::FACE_NOT_EXIST" << std::endl;
        return false;
    }
    //std::cout << "delete face: " << face->incEdge->origin->id << " " << face->incEdge->succ->origin->id << " " << face->incEdge->prev->origin->id << std::endl;
    auto edge = face->incEdge;
    edge->origin->Q -= face->Kp;
    edge->succ->origin->Q -= face->Kp;
    edge->prev->origin->Q -= face->Kp;

    
    delete_halfedge(edge->succ);
    delete_halfedge(edge->prev);
    delete_halfedge(edge);
    auto iter = std::find(m_faces.begin(), m_faces.end(), face);
    if (iter != m_faces.end())
    {
        m_faces.erase(iter);
        delete face;
        return true;
    }
    return false;
}

bool Mesh::delete_halfedge(HalfEdge* he)
{
    if (he == nullptr)
    {
        std::cerr << "ERROR::MESH::DELETE_HALFEDGE::EDGE_NOT_EXIST" << std::endl;
        return false;
    }
    auto iter = std::find(m_edges.begin(), m_edges.end(), he);
    
    if (iter != m_edges.end())
    {
        
        m_edges.erase(iter);
        delete he; 
        return true;
    }
    return false;
}

bool Mesh::delete_vertex(Vertex* v)
{
    //std::cout << "delete vertex: " << v->id << std::endl;
    if (v == nullptr)
    {
        std::cerr << "ERROR::MESH::DELETE_VERTEX::VERTEX_NOT_EXIST" << std::endl;
        return false;
    }
    auto iter = std::find(m_vertexes.begin(), m_vertexes.end(), v);
    if (iter != m_vertexes.end())
    {
        m_vertexes.erase(iter);
        delete v;
        return true;
    }
    return false;
}

bool Mesh::has(Vertex* v)
{
    return std::find(m_vertexes.begin(), m_vertexes.end(), v) != m_vertexes.end();
}

bool Mesh::has(HalfEdge* he)
{
    return std::find(m_edges.begin(), m_edges.end(), he) != m_edges.end();
}

bool Mesh::has(Face* face)
{
    return std::find(m_faces.begin(), m_faces.end(), face) != m_faces.end();
}

std::vector<HalfEdge*> Mesh::get_incEdge(Vertex* v)
{
    auto start_he = v->incEdge;
    auto he = start_he;
    std::vector<HalfEdge*> res;
    if (!has(v)) 
        return res;

    //std::cout << "v_id: " << v->id << " ";
    do
    {
        if (he->incFace == nullptr) break;
        res.push_back(he);
        he = he->prev->twin;
    } while (he != start_he);
    if (he->incFace == nullptr)
    {
        
        res.clear(); 
        do
        {
            res.push_back(he);
            he = he->twin->succ;
        } while (he->twin->incFace != nullptr);
        res.push_back(he);
        
    }
    //std::cout << "res: ";
    //for (auto& i : res) std::cout << "->" << i->twin->origin->id << " ";
    //std::cout << std::endl;
    return res;
}

std::vector<Vertex*> Mesh::get_neighbor_vertex(Vertex* v)
{
    auto incEdges = Mesh::get_incEdge(v);
    std::vector<Vertex*> res;
    for (auto& e : incEdges)
    {
        if (e->twin != nullptr)
            res.push_back(e->twin->origin);
    }
    return res;
}

bool Mesh::load_file(std::string obj_path)
{
    clear();
    if (!objloader->load_file(obj_path))
    {
        return false;
    }
    m_GLvertexes = objloader->vertexes;
    for (int i = 0; i < m_GLvertexes.size(); i++)
    {
        createVertex(m_GLvertexes[i].position);
    }
    for (int i = 0; i < objloader->indices.size(); i++)
    {
        auto indice = objloader->indices[i];
        Vertex* verts[3];
        for (int j = 0; j < 3; j++)
        {
            m_GLindices.push_back(indice[j]);
            verts[j] = m_vertexes[indice[j]];
        }
        createFace(verts);
    }
    //flag_boundary();
    std::cout << "[Mesh Loader] Triangle number: "  << m_GLindices.size() / 3.0 << std::endl 
        << "Vertex number: " << m_GLvertexes.size() << std::endl;

    auto_normal();
    setupGL();
    return true;
}

void Mesh::auto_normal()
{
    for (int i = 0; i < m_GLindices.size(); i += 3)
    {
        glm::vec3 vecA = m_GLvertexes[m_GLindices[i + 1]].position - m_GLvertexes[m_GLindices[i]].position;
        glm::vec3 vecB = m_GLvertexes[m_GLindices[i + 2]].position - m_GLvertexes[m_GLindices[i + 1]].position;
        vecA = glm::normalize(vecA);
        vecB = glm::normalize(vecB);
        glm::vec3 face_norm = glm::cross(vecA, vecB);
        m_GLvertexes[m_GLindices[i]].normal += face_norm;
        m_GLvertexes[m_GLindices[i + 1]].normal += face_norm;
        m_GLvertexes[m_GLindices[i + 2]].normal += face_norm;
    }
    for (int i = 0; i < m_GLvertexes.size(); i++)
    {
        m_GLvertexes[i].normal = glm::normalize(m_GLvertexes[i].normal);
    }
}

void Mesh::draw(const Shader& shader)
{
    shader.setVec3("color", 0.9f, 0.9f, 0.9f);
    shader.setMat4("model", GetModelMatrix());
    shader.setVec3("diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("ambient", 0.4f, 0.4f, 0.4f);
    shader.setVec3("specular", 0.4f, 0.4f, 0.4f);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_TRI); 
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, m_GLindices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


glm::mat4 Mesh::GetModelMatrix()
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(m_mesh_pos[0], m_mesh_pos[1], m_mesh_pos[2])); // translate it down so it's at the center of the scene
    model = glm::rotate(model, m_mesh_rot[0], glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, m_mesh_rot[1], glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, m_mesh_rot[2], glm::vec3(0.0f, 0.0f, 1.0f));
    return model;
}

void Mesh::clear()
{
    m_GLvertexes.clear();
    m_GLindices.clear();
    util::pointerVectorClear<Vertex*>(m_vertexes);
    util::pointerVectorClear<HalfEdge*>(m_edges);
    util::pointerVectorClear<Face*>(m_faces);
}

void Mesh::setupGL()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO_TRI);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_TRI);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_GLindices.size() * sizeof(GLuint), &m_GLindices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Binding and setting VAO
    glBindVertexArray(VAO);

    // Buffering data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_GLvertexes.size() * sizeof(GLVertex), &m_GLvertexes[0], GL_STATIC_DRAW);
    
    // Configuring attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (void*)offsetof(GLVertex, normal));

    // Unbinding VAO
    glBindVertexArray(0);
}

void Mesh::updateGL()
{
    if (VBO == 0) setupGL(); //setup first
    m_GLvertexes.clear();
    m_GLindices.clear();
    std::unordered_map<int, int> idx_map;
    for (auto& vertex : m_vertexes)
    {
        idx_map[vertex->id] = m_GLvertexes.size();
        m_GLvertexes.push_back(GLVertex(vertex->position));
    }

    for (auto& face : m_faces)
    {
        m_GLindices.push_back(idx_map[face->incEdge->origin->id]);
        m_GLindices.push_back(idx_map[face->incEdge->succ->origin->id]);
        m_GLindices.push_back(idx_map[face->incEdge->prev->origin->id]);
    }
    //std::cout << m_GLindices.size() << " " << m_GLvertexes.size() << std::endl;
    auto_normal();

    glBindVertexArray(VAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_TRI);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_GLindices.size() * sizeof(GLuint), &m_GLindices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, m_GLvertexes.size() * sizeof(GLVertex), &m_GLvertexes[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (void*)offsetof(GLVertex, normal));

    glBindVertexArray(0);
}
