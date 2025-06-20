#pragma once
#include "Mesh.h"
#include <utility>
#include <map>
#include <queue>
#include <algorithm>

struct VerticesPair
{
    Vertex* v1 = nullptr;
    Vertex* v2 = nullptr;
    glm::vec3 v_bar;
    float cost = 0.f;
    int timestamp = -1;
    VerticesPair(Vertex* v1, Vertex* v2, float cost, glm::vec3 v_bar, int timestamp = 0) 
        :v1(v1), v2(v2), cost(cost), v_bar(v_bar), timestamp(timestamp) {}
    bool operator < (const VerticesPair& a) const
    {
        return cost > a.cost;
    }
};

bool cmp_vertex(Vertex* v1, Vertex* v2);

class Simplification
{
public:
	Simplification(Mesh* mesh);
	~Simplification();

	void step();

    void reset();

    void add_pair_to_heap(Vertex* v1, Vertex* v2, int timestamp);

    void edge_collaspe(Vertex* v1, Vertex* v2, const glm::vec3& pos);

    void set_reduction_ratio(float ratio);

    void init(float ratio = 0.8);

    bool is_collaspe_ok(Vertex* v1, Vertex* v2);

    bool is_finished();

    float rate_of_process();

    VerticesPair cal_cost(Vertex* v1, Vertex* v2) const;

    float total_cost = 0.f;

private:
	Mesh* mesh;
    std::priority_queue<VerticesPair> cost_heap;
    std::map<std::pair<int,int>, int> edge_timestamp;   //record the newest timestamp of edge
    int iter_num = 0;
    int epoch = 0;
    
    float NEIGHBOARD_THRSHOLD = 0.01;
    float REDUCTION_RATIO = 0.5;
};

