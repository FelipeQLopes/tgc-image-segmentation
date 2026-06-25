#include "../inc/graphDir.hpp"

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <cmath>
#include <iostream>
#include <list>
#include <stack>
#include <vector>

Graph::Graph(int numVertices)
{
    vertexes.resize(numVertices);
    listaAdjacencia.resize(numVertices);

    for (int i = 0; i < numVertices; i++){
        vertexes[i] = i;
    }
}

std::string Graph::toString(){
    std::stringstream ss;
    
    for (size_t i = 0; i < listaAdjacencia.size(); i++)
    {   
        ss << i << ": ";
        for (Edge e : listaAdjacencia[i])
        {
            ss << "(" << e.u << " -> " << e.v << ") w="<<e.w << " | ";
        }
        ss << "\n";
    }

    return ss.str();
}

void Graph::addEdge(int u, int v, int w){
    listaAdjacencia[u].push_back(Edge(u,v,w));
}

#define NIL -1

void Graph::SCCUtil(int u, int disc[], int low[],
                    std::stack<int>* st, bool stackMember[],
                    std::vector<std::vector<int>>& result)
{
    static int time = 0;

    disc[u] = low[u] = ++time;
    st->push(u);
    stackMember[u] = true;

    for (auto &e : listaAdjacencia[u]) {
        int v = e.v;

        if (disc[v] == NIL) {
            SCCUtil(v, disc, low, st, stackMember, result);
            low[u] = std::min(low[u], low[v]);
        }
        else if (stackMember[v]) {
            low[u] = std::min(low[u], disc[v]);
        }
    }

    // Se u é a raiz da SCC
    if (low[u] == disc[u]) {
        std::vector<int> component;

        while (st->top() != u) {
            int w = st->top();
            component.push_back(w);
            stackMember[w] = false;
            st->pop();
        }

        int w = st->top();
        component.push_back(w);
        stackMember[w] = false;
        st->pop();

        result.push_back(component);
    }
}

std::vector<std::vector<int>> Graph::SCC()
{
    int V = vertexes.size();

    int* disc = new int[V];
    int* low = new int[V];
    bool* stackMember = new bool[V];
    std::stack<int>* st = new std::stack<int>();

    std::vector<std::vector<int>> result;

    for (int i = 0; i < V; i++) {
        disc[i] = NIL;
        low[i] = NIL;
        stackMember[i] = false;
    }

    for (int i = 0; i < V; i++)
        if (disc[i] == NIL)
            SCCUtil(i, disc, low, st, stackMember, result);

    delete[] disc;
    delete[] low;
    delete[] stackMember;
    delete st;

    return result;
}

Graph Graph::optimumBranchingGabow(int root) {
    int V = vertexes.size();
    std::vector<Edge> inEdge(V, Edge(-1, -1, std::numeric_limits<double>::max()));

    for(int u = 0; u < V; u++) {
        for(auto &e : listaAdjacencia[u]) {
            if(e.v != root && e.w < inEdge[e.v].w) inEdge[e.v] = e;
        }
    }

    std::vector<int> visited(V, -1);
    std::vector<int> cycle(V, -1);
    int cycleId = 0;

    for(int i = 0; i < V; i++) {
    if(i == root) continue;
    int v = i;
    while(v != root && visited[v] == -1 && inEdge[v].u != -1) {
        visited[v] = i;
        v = inEdge[v].u;
    }
    if(v != root && visited[v] == i) {
        int u = v;
        do {
            cycle[u] = cycleId;
            u = inEdge[u].u;
        } while(u != v);
        cycleId++;
    }
}


    bool hasCycle = false;
    for(int i = 0; i < V; i++) if(cycle[i] != -1) hasCycle = true;
    if(!hasCycle) {
        Graph res(V);
        for(int i = 0; i < V; i++) {
            if(i == root) continue;
            if(inEdge[i].u != -1) res.addEdge(inEdge[i].u, inEdge[i].v, inEdge[i].w);
        }
        return res;
    }

    int newV = V - cycleId;
    std::vector<int> newVertex(V, -1);
    int idx = 0;
    for(int i = 0; i < V; i++) if(cycle[i] == -1) newVertex[i] = idx++;
    std::vector<int> cycleMap(cycleId, idx++);
    for(int i = 0; i < V; i++) if(cycle[i] != -1) newVertex[i] = cycleMap[cycle[i]];

    Graph contracted(idx);
    for(int u = 0; u < V; u++) {
        for(auto &e : listaAdjacencia[u]) {
            int cu = newVertex[u], cv = newVertex[e.v];
            if(cu != cv) contracted.addEdge(cu, cv, e.w - ((cycle[e.v] != -1) ? inEdge[e.v].w : 0));
        }
    }

    Graph contractedRes = contracted.optimumBranchingGabow(newVertex[root]);

    Graph result(V);
    for(int u = 0; u < V; u++) {
        if(inEdge[u].u != -1 && cycle[u] == -1) result.addEdge(inEdge[u].u, inEdge[u].v, inEdge[u].w);
    }
    for(int u = 0; u < V; u++) {
        if(cycle[u] != -1) result.addEdge(inEdge[u].u, inEdge[u].v, inEdge[u].w);
    }

    return result;
}

Graph Graph::optimumBranchingTarjan(int root) {
    int V = vertexes.size();
    std::vector<Edge> inEdge(V, Edge(-1, -1, std::numeric_limits<double>::max()));

    for (int u = 0; u < V; u++) {
        for (auto &e : listaAdjacencia[u]) {
            if (e.v != root && e.w < inEdge[e.v].w) {
                inEdge[e.v] = e;
            }
        }
    }

    std::vector<int> visited(V, -1), cycle(V, -1);
    int cycleId = 0;

    for (int i = 0; i < V; i++) {
        if (i == root) continue;
        int v = i;
        while (v != root && visited[v] == -1 && inEdge[v].u != -1) {
            visited[v] = i;
            v = inEdge[v].u;
        }
        if (v != root && visited[v] == i) { 
            int u = v;
            do {
                cycle[u] = cycleId;
                u = inEdge[u].u;
            } while (u != v);
            cycleId++;
        }
    }

    bool hasCycle = false;
    for (int i = 0; i < V; i++) if (cycle[i] != -1) hasCycle = true;
    if (!hasCycle) {
        Graph res(V);
        for (int i = 0; i < V; i++) {
            if (i == root) continue;
            if (inEdge[i].u != -1) res.addEdge(inEdge[i].u, inEdge[i].v, inEdge[i].w);
        }
        return res;
    }

    int newV = V - cycleId;
    std::vector<int> newVertex(V, -1);
    int idx = 0;
    for (int i = 0; i < V; i++) if (cycle[i] == -1) newVertex[i] = idx++;
    std::vector<int> cycleMap(cycleId, idx++);
    for (int i = 0; i < V; i++) if (cycle[i] != -1) newVertex[i] = cycleMap[cycle[i]];

    Graph contracted(idx);
    for (int u = 0; u < V; u++) {
        for (auto &e : listaAdjacencia[u]) {
            int cu = newVertex[u], cv = newVertex[e.v];
            if (cu != cv) {
                double w = e.w;
                if (cycle[e.v] != -1) w -= inEdge[e.v].w; 
                contracted.addEdge(cu, cv, w);
            }
        }
    }

    Graph contractedRes = contracted.optimumBranchingTarjan(newVertex[root]);

    Graph result(V);
    for (int u = 0; u < V; u++) {
        if (inEdge[u].u != -1 && cycle[u] == -1) {
            result.addEdge(inEdge[u].u, inEdge[u].v, inEdge[u].w);
        }
    }
    for (int u = 0; u < V; u++) {
        if (cycle[u] != -1) {
            result.addEdge(inEdge[u].u, inEdge[u].v, inEdge[u].w);
        }
    }

    return result;
}

struct UnionFind {
    std::vector<int> parent, rank;

    UnionFind(int n){
        parent.resize(n);
        rank.resize(n, 0);
        for(int i = 0; i < n; i++)
            parent[i] = i;
    }

    int find(int x){
        if(parent[x] != x)
            parent[x] = find(parent[x]); 
        return parent[x];
    }

    void unite(int a, int b){
        a = find(a);
        b = find(b);
        if(a == b) return;

        if(rank[a] < rank[b]) parent[a] = b;
        else if(rank[a] > rank[b]) parent[b] = a;
        else{
            parent[b] = a;
            rank[a]++;
        }
    }
};


std::vector<int> Graph::componentesFracamenteConexos() {

    int V = vertexes.size();
    UnionFind uf(V);

    for(int u = 0; u < V; u++){
        for(auto &e : listaAdjacencia[u]){
            uf.unite(u, e.v);
        }
    }

    std::vector<int> comp(V);
    for(int i = 0; i < V; i++)
        comp[i] = uf.find(i);

    return comp;
}

