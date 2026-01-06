#include "Tarjan.h"

int main()
{
    G.assign(n, {});
    is_bridge.assign(10, false); // 7 条边，留点余量

    // 构图
    add_edge(0, 1); // 0
    add_edge(1, 2); // 1
    add_edge(2, 3); // 2
    add_edge(3, 1); // 3
    add_edge(2, 4); // 4
    add_edge(4, 3); // 5
    add_edge(4, 5); // 6

    // 1. Tarjan（桥 + 割点）
    tarjan_undirected();

    // 输出桥
    cout << "Bridges:\n";
    for (int i = 0; i < edge_cnt; ++i)
        if (is_bridge[i])
            cout << "edge " << i << " is bridge\n";

    // 输出割点
    cout << "\nArticulation Points:\n";
    for (int i = 0; i < n; ++i)
        if (is_cut[i])
            cout << "node " << i << " is cut\n";

    // 2. 边双
    build_ebcc();

    cout << "\nEBCC:\n";
    for (int i = 0; i < n; ++i)
        cout << "node " << i << " -> ebcc " << ebcc_id[i] << "\n";

    // 3. block graph
    build_block_graph();

    cout << "\nBlock Graph:\n";
    for (int i = 0; i < (int)block_graph.size(); ++i)
    {
        cout << "block " << i << ": ";
        for (int v : block_graph[i])
            cout << v << " ";
        cout << "\n";
    }

    return 0;
}
