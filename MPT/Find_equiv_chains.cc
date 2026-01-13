#include <unordered_map>
#include <unordered_set>
#include <vector>
using Graph = std::unordered_map<int, std::unordered_set<int>>;


bool has_edge(const map<int, map<int,int>>& graph, int u, int v) {
    auto it = graph.find(u);
    if (it == graph.end()) return false;
    return it->second.count(v);
}

void find_structures_4(
    const map<int, map<int,int>>& graph,
    map<int, set<int>>& equiv_chain,
    set<set<int>>& kn_ids
) {
    // 枚举边 (x, y)
    for (auto itx = graph.begin(); itx != graph.end(); ++itx) {
        int x = itx->first;
        const auto& nx = itx->second;

        for (const auto& [y, _] : nx) {
            if (y <= x) continue;

            const auto& ny = graph.at(y);

            // 找 z，使得 x,y,z 构成三角形
            const auto& small_xy = (nx.size() < ny.size()) ? nx : ny;
            const auto& large_xy = (nx.size() < ny.size()) ? ny : nx;

            for (const auto& [z, __] : small_xy) {
                if (z <= y) continue;
                if (!large_xy.count(z)) continue;

                // {x,y,z} 是一个 clique

                const auto& nz = graph.at(z);

                // 求公共邻居 S = N(x) ∩ N(y) ∩ N(z)
                vector<int> S;
                const auto& small_xyz =
                    (nx.size() <= ny.size() && nx.size() <= nz.size()) ? nx :
                    (ny.size() <= nx.size() && ny.size() <= nz.size()) ? ny : nz;

                for (const auto& [v, ___] : small_xyz) {
                    if (v == x || v == y || v == z) continue;
                    if (has_edge(graph, x, v) &&
                        has_edge(graph, y, v) &&
                        has_edge(graph, z, v)) {
                        S.push_back(v);
                    }
                }

                // 在 S 中两两枚举 (a,b)
                for (size_t i = 0; i < S.size(); ++i) {
                    for (size_t j = i + 1; j < S.size(); ++j) {
                        int a = S[i];
                        int b = S[j];

                        if (!has_edge(graph, a, b)) {
                            // 情况 A：无边 → equiv_chain
                            equiv_chain[a].insert(b);
                            equiv_chain[b].insert(a);
                        } else {
                            // 情况 B：有边 → kn_ids
                            set<int> k_n = {a, b, x, y, z};
                            kn_ids.insert(k_n);
                        }
                    }
                }
            }
        }
    }
}

void find_diamond_structure(
    const Graph& graph,
    std::map<int, std::set<int>>& equiv_chain,
    std::set<std::set<int>>& kn_ids
) {
    // 枚举中间边 (x, y)
    for (auto itx = graph.begin(); itx != graph.end(); ++itx) {
        int x = itx->first;
        const auto& nx = itx->second;

        for (const auto& [y, _] : nx) {
            if (y <= x) continue;  // 保证 (x,y) 只处理一次

            const auto& ny = graph.at(y);

            // 计算公共邻居 S = N(x) ∩ N(y)
            std::vector<int> S;
            const auto& small = (nx.size() < ny.size()) ? nx : ny;
            const auto& large = (nx.size() < ny.size()) ? ny : nx;

            for (const auto& [v, __] : small) {
                if (v == x || v == y) continue;
                if (large.count(v)) {
                    S.push_back(v);
                }
            }

            // 在公共邻居中枚举 (a, b)
            for (size_t i = 0; i < S.size(); ++i) {
                for (size_t j = i + 1; j < S.size(); ++j) {
                    int a = S[i];
                    int b = S[j];

                    if (!has_edge(graph, a, b)) {
                        // 情况 1：a-b 无边
                        equiv_chain[a].insert(b);
                        equiv_chain[b].insert(a);
                    } else {
                        // 情况 2：a-b 有边 → 形成 4-clique
                        std::set<int> k_n = {a, b, x, y};
                        kn_ids.insert(k_n);
                    }
                }
            }
        }
    }
}


