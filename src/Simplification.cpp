#include "Simplification.h"

Simplification::Simplification(Mesh* mesh): mesh(mesh)
{
	;
}

Simplification::~Simplification()
{
	;
}

void Simplification::step()
{
    if (epoch >= iter_num) return;
    epoch++;
    auto v_pair = cost_heap.top();
    while (v_pair.timestamp < edge_timestamp[std::make_pair(v_pair.v1->id, v_pair.v2->id)] ||
        !is_collaspe_ok(v_pair.v1, v_pair.v2))
    {
        cost_heap.pop();
        if (cost_heap.empty()) return;
        v_pair = cost_heap.top();
    }
    auto bias = glm::length(v_pair.v_bar - v_pair.v1->position);
    edge_collaspe(v_pair.v1, v_pair.v2, v_pair.v_bar);
    cost_heap.pop();
    total_cost += v_pair.cost;

    for (auto& edge : mesh->get_incEdge(v_pair.v1))
    {
        Vertex* to = edge->twin->origin;
        add_pair_to_heap(v_pair.v1, to, epoch);
    }
    //std::cout << "[Simplification] Simplified Face Number: " << mesh->m_faces.size() << "  Total Cost: " << total_cost << std::endl;
}
void Simplification::reset()
{
    while(!cost_heap.empty())cost_heap.pop();
    edge_timestamp.clear();
}

void Simplification::add_pair_to_heap(Vertex* v1, Vertex* v2, int timestamp)
{
    auto pair_t = cal_cost(v1, v2);
    pair_t.timestamp = timestamp;
    //std::cout << "(" << v1->id << "," << v2->id << ") -> " << pair_t.cost << std::endl;
    edge_timestamp[std::make_pair(v1->id, v2->id)] = timestamp;
    edge_timestamp[std::make_pair(v2->id, v1->id)] = timestamp;
    cost_heap.push(pair_t);
}

void Simplification::edge_collaspe(Vertex* v1, Vertex* v2, const glm::vec3& pos)
{
    // Remove v2 and set v1 to pos
    if (!mesh->has(v1) || !mesh->has(v2))return;
    auto incEdges = mesh->get_incEdge(v2);
    for (auto& edge : incEdges)
    {
        // exist edge from v2 to v1
        if (edge->twin->origin == v1)
        {
            // process face 1
            if (edge->twin->incFace != nullptr)
            {
                //if (v1->incEdge == edge->twin) v1->incEdge = edge->twin->prev->twin;
                if (edge->twin->prev->origin->incEdge == edge->twin->prev)
                {
                    edge->twin->prev->origin->incEdge = edge->twin->succ->twin;
                }

                edge->twin->prev->twin->twin = edge->twin->succ->twin;
                edge->twin->succ->twin->twin = edge->twin->prev->twin;
                mesh->delete_face(edge->twin->incFace);
            }
            else
            {
                mesh->delete_halfedge(edge->twin);
            }
            // process face2
            if (edge->incFace != nullptr)
            {
                //if (v1->incEdge == edge->succ) v1->incEdge = edge->prev->twin;
                if (edge->prev->origin->incEdge == edge->prev)
                {
                    edge->prev->origin->incEdge = edge->succ->twin;
                }
                edge->succ->twin->twin = edge->prev->twin;
                edge->prev->twin->twin = edge->succ->twin;
                mesh->delete_face(edge->incFace);
            }
            else
            {
                mesh->delete_halfedge(edge);
            }
            break;
        }
    }
    for (auto& edge : incEdges)
    {
        if (!mesh->has(edge))
        {
            continue;
        }
        v1->incEdge = edge;
        edge->origin = v1;
        if (edge->incFace != nullptr)v1->Q += edge->incFace->Kp;
    }
    v1->position = pos;
    mesh->delete_vertex(v2);
}

void Simplification::set_reduction_ratio(float ratio)
{
    REDUCTION_RATIO = ratio;
}

void Simplification::init(float ratio)
{
    set_reduction_ratio(ratio);
    iter_num = mesh->m_faces.size() * 0.5 * REDUCTION_RATIO;
    epoch = 0;
    total_cost = 0.f;
    reset();
    for (int i = 0; i < mesh->m_edges.size(); i++)
    {
        Vertex* v1 = mesh->m_edges[i]->origin;
        Vertex* v2 = mesh->m_edges[i]->twin->origin;
        add_pair_to_heap(v1, v2, 0);
    }
    //std::sort(mesh->m_vertexes.begin(), mesh->m_vertexes.end(), cmp_vertex);
    //for (int i = 0; i < mesh->m_vertexes.size(); i++)
    //{
    //    for (int j = i + 1; j < mesh->m_vertexes.size(); j++)
    //    {
    //        if (mesh->m_vertexes[j]->position.x - mesh->m_vertexes[i]->position.x >= NEIGHBOARD_THRSHOLD) break;
    //        if (glm::length(mesh->m_vertexes[i]->position - mesh->m_vertexes[j]->position) < NEIGHBOARD_THRSHOLD)
    //        {
    //            add_pair_to_heap(mesh->m_vertexes[i], mesh->m_vertexes[j], 0);
    //        }
    //    }
    //}

    std::cout << "[Simplification] Initial Face Number: " << mesh->m_faces.size()
        << "  Valid Pairs Number:" << cost_heap.size() << std::endl;
}

bool Simplification::is_collaspe_ok(Vertex* v1, Vertex* v2)
{
    if (!mesh->has(v1) || !mesh->has(v2))return false;
    auto v1_incEdges = mesh->get_incEdge(v1);
    HalfEdge* v12 = nullptr;
    for (auto& e : v1_incEdges)
    {
        if (!mesh->has(e) || !mesh->has(e->twin))return false;
        if (e->twin->origin == v2)
        {
            v12 = e;
            break;
        }
    }
    if (v12 == nullptr)
    {
        return false;
    }
    Vertex* vl = nullptr;
    Vertex* vr = nullptr;
    if (v12->incFace != nullptr)
    {
        if (!mesh->has(v12->succ->twin) || !mesh->has(v12->prev->twin) 
            || v12->succ->twin->incFace == nullptr || v12->prev->twin->incFace == nullptr)
            return false;
        vl = v12->succ->twin->origin;
    }
    if (v12->twin->incFace != nullptr)
    {
        if (!mesh->has(v12->twin->succ->twin) || !mesh->has(v12->twin->prev->twin)
            || v12->twin->succ->twin->incFace == nullptr || v12->twin->prev->twin->incFace == nullptr)
            return false;
        vr = v12->twin->succ->twin->origin;
    }
    if (vl == vr)
        return false;
    
    auto v1_neighbor = mesh->get_neighbor_vertex(v1);
    auto v2_neighbor = mesh->get_neighbor_vertex(v2);
    for (auto& v : v1_neighbor)
    {
        if (std::find(v2_neighbor.begin(), v2_neighbor.end(), v) != v2_neighbor.end())
        {
            // intersection
            if (v != vl && v != vr)
            {
                return false;
            }
        }
    }
    return true;
}

bool Simplification::is_finished()
{
    return epoch >= iter_num;
}

float Simplification::rate_of_process()
{
    if (iter_num == 0 || epoch == 0) return 0.f;
    return  epoch / (float)iter_num;
}

VerticesPair Simplification::cal_cost(Vertex* v1, Vertex* v2) const
{
    glm::mat4 q_bar = v1->Q + v2->Q;
    glm::mat4 q_bar_grad(
        q_bar[0],
        q_bar[1],
        q_bar[2],
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    q_bar_grad = glm::transpose(q_bar_grad);
    if (fabs(glm::determinant(q_bar_grad)) > 1e-4)
    {
        glm::vec4 v_bar = glm::inverse(q_bar_grad) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float cost = glm::dot(v_bar, q_bar * v_bar);
        return VerticesPair(v1, v2, cost, glm::vec3(v_bar));
    }
    else
    {
        glm::vec4 v_mid(0.5f * (v1->position + v2->position), 1.0f);
        float cost = glm::dot(v_mid, q_bar * v_mid);
        return VerticesPair(v1, v2, cost, glm::vec3(v_mid));
    }
}



bool cmp_vertex(Vertex* v1, Vertex* v2)
{
    if (v1->position.x < v2->position.x)
    {
        return true;
    }
    else if (v1->position.y < v2->position.y)
    {
        return true;
    }
    else return v1->position.z < v2->position.z;
}
