#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

// ---------- 并查集 ----------
struct DSU {
    vector<int> p, sz;
    DSU(int n = 0) { init(n); }
    void init(int n) {
        p.resize(n);
        iota(p.begin(), p.end(), 0);
        sz.assign(n, 1);
    }
    int find(int x) {
        while (p[x] != x) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }
    int unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return a;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
        return a;
    }
    int size(int x) { return sz[find(x)]; }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int64 n_est, m;
    cin >> n_est >> m;

    vector<tuple<int64, int64, int>> edges;
    edges.reserve(m);
    unordered_set<int64> ids;
    ids.reserve(m * 2);

    for (int64 i = 0; i < m; ++i) {
        int64 u, v; int w;
        cin >> u >> v >> w;
        edges.emplace_back(u, v, w);
        ids.insert(u); ids.insert(v);
    }

    // 压缩 ID
    vector<int64> uniq(ids.begin(), ids.end());
    sort(uniq.begin(), uniq.end());
    unordered_map<int64, int> idmap;
    for (size_t i = 0; i < uniq.size(); ++i)
        idmap[uniq[i]] = (int)i;
    int N = uniq.size();

    // 按权重分组
    map<int, vector<pair<int, int>>> by_weight;
    for (auto &[u0, v0, w] : edges)
        by_weight[w].push_back({idmap[u0], idmap[v0]});
    edges.clear();

    DSU dsu(N);
    vector<vector<int>> adj_raw(N), adj_sorted(N);
    vector<int> last_ver(N, -1);
    int ver = 0;

    auto normalize = [&](int u) {
        if (last_ver[u] == ver) return;
        vector<int> tmp;
        tmp.reserve(adj_raw[u].size());
        for (int x : adj_raw[u]) {
            int rx = dsu.find(x);
            if (rx != u) tmp.push_back(rx);
        }
        sort(tmp.begin(), tmp.end());
        tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());
        adj_sorted[u].swap(tmp);
        last_ver[u] = ver;
    };

    auto merge_nodes = [&](int a, int b) {
        a = dsu.find(a); b = dsu.find(b);
        if (a == b) return a;
        if (dsu.size(a) < dsu.size(b)) swap(a, b);
        adj_raw[a].insert(adj_raw[a].end(), adj_raw[b].begin(), adj_raw[b].end());
        adj_raw[b].clear();
        last_ver[a] = last_ver[b] = -1;
        return dsu.unite(a, b);
    };

    for (auto &[w, elist] : by_weight) {
        cerr << "Processing weight = " << w << "\n";

        unordered_set<int> affected;
        for (auto &[u0, v0] : elist) {
            int u = dsu.find(u0);
            int v = dsu.find(v0);
            if (u == v) continue;
            adj_raw[u].push_back(v);
            adj_raw[v].push_back(u);
            affected.insert(u);
            affected.insert(v);
        }

        // 层内重复检测直到稳定
        bool changed = true;
        while (changed) {
            changed = false;
            ver++;
            vector<int> q(affected.begin(), affected.end());
            affected.clear();
            size_t qi = 0;
            while (qi < q.size()) {
                int u = dsu.find(q[qi++]);
                normalize(u);
                vector<int> neighbors = adj_sorted[u];
                for (int raw_v : neighbors) {
                    int v = dsu.find(raw_v);
                    if (u == v) continue;
                    normalize(u);
                    normalize(v);
                    auto &A = adj_sorted[u];
                    auto &B = adj_sorted[v];
                    if (A.size() < 2 || B.size() < 2) continue;

                    // 交集
                    vector<int> common;
                    size_t i = 0, j = 0;
                    while (i < A.size() && j < B.size()) {
                        if (A[i] < B[j]) i++;
                        else if (A[i] > B[j]) j++;
                        else {
                            common.push_back(A[i]);
                            i++; j++;
                        }
                    }

                    if (common.size() >= 2) {
                        int root = common[0];
                        for (size_t k = 1; k < common.size(); ++k)
                            root = merge_nodes(root, common[k]);
                        int rep = dsu.find(root);
                        q.push_back(rep);
                        affected.insert(rep);
                        changed = true;
                    }
                }
            }
        }
    }

    // 输出结果
    unordered_map<int, vector<int>> groups;
    for (int i = 0; i < N; ++i)
        groups[dsu.find(i)].push_back(i);

    cout << "Total merged groups: " << groups.size() << "\n";
    for (auto &[r, vs] : groups) {
        cout << "Group rep " << uniq[r] << " (size=" << vs.size() << "): ";
        for (int v : vs) cout << uniq[v] << " ";
        cout << "\n";
    }
}
