#ifndef TARJAN_H
#define TARJAN_H
#include <iostream>
#include <unordered_map>
#include <vector>
#include <climits>
#include <algorithm>
#include <stack>
using namespace std;

struct Edge {
    int to;
    int id;
};

int n = 6;
vector<vector<Edge>> G;
vector<bool> is_bridge;

int edge_cnt = 0;

void add_edge(int u, int v)
{
    G[u].push_back({v, edge_cnt});
    G[v].push_back({u, edge_cnt});
    edge_cnt++;
}

vector<int> dfn, low;
vector<bool> is_cut;

int ts = 0;
struct Frame {
    int u;          // 当前节点
    int parent_eid; // 来自父节点的边 id
    int ei;         // 当前处理到第几条邻边
    int child;      // DFS 子树数量（割点用）
};

void tarjan_undirected()
{
    dfn.assign(n, 0);
    low.assign(n, 0);
    is_cut.assign(n, false);
    ts = 0;

    stack<Frame> st;

    for (int start = 0; start < n; ++start)
    {
        if (dfn[start]) continue;

        st.push({start, -1, 0, 0});

        while (!st.empty())
        {
            Frame &f = st.top();
            int u = f.u;

            // 第一次访问
            if (f.ei == 0)
            {
                dfn[u] = low[u] = ++ts;
            }

            if (f.ei < (int)G[u].size())
            {
                auto tmp = G[u][f.ei++];
                int v   = tmp.to;
                int eid = tmp.id;

                if (eid == f.parent_eid)
                    continue;

                if (!dfn[v])
                {
                    f.child++;
                    st.push({v, eid, 0, 0});
                }
                else
                {
                    // 回边
                    low[u] = min(low[u], dfn[v]);
                }
            }
            else
            {
                // 所有邻边处理完，开始“回溯”
                st.pop();

                if (f.parent_eid != -1)
                {
                    int parent = st.top().u;
                    low[parent] = min(low[parent], low[u]);

                    // 桥
                    if (low[u] > dfn[parent])
                        is_bridge[f.parent_eid] = true;

                    // 割点（非根）
                    Frame &pf = st.top();
                    if (pf.parent_eid != -1 && low[u] >= dfn[parent])
                        is_cut[parent] = true;
                }
                else
                {
                    // 根节点割点判定
                    if (f.child >= 2)
                        is_cut[u] = true;
                }
            }
        }
    }
}

vector<int> ebcc_id;
int ebcc_cnt = 0;

void build_ebcc()
{
    ebcc_id.assign(n, -1);
    ebcc_cnt = 0;

    for (int i = 0; i < n; ++i)
    {
        if (ebcc_id[i] != -1) continue;

        stack<int> st;
        st.push(i);
        ebcc_id[i] = ebcc_cnt;

        while (!st.empty())
        {
            int u = st.top(); st.pop();
            for (auto& tmp : G[u])
            {
                const auto& v = tmp.to;
                const auto& eid = tmp.id;
                if (is_bridge[eid]) continue;
                if (ebcc_id[v] == -1)
                {
                    ebcc_id[v] = ebcc_cnt;
                    st.push(v);
                }
            }
        }

        ++ebcc_cnt;
    }
}


vector<vector<int>> block_graph;
void build_block_graph()
{
    block_graph.assign(ebcc_cnt, {});

    for (int u = 0; u < n; ++u)
    {
        for (auto& tmp : G[u])
        {
            const auto& v = tmp.to;
            const auto& eid = tmp.id;
            if (is_bridge[eid])
            {
                int a = ebcc_id[u];
                int b = ebcc_id[v];
                if (a != b)
                {
                    block_graph[a].push_back(b);
                }
            }
        }
    }
}




#endif