#include <iostream>
#include <unordered_map>
#include <vector>
#include <climits>
#include <algorithm>

// Define the graph type using unordered_map
using Graph = std::unordered_map<int, std::unordered_map<int, int>>;
using namespace std;

// Stoer-Wagner algorithm to find the minimum cut of a graph
std::pair<int, std::pair<std::vector<int>, std::vector<int>>> stoerWagner(Graph& graph) {
    // std::vector<int> minimal_cut_group;
    int n = graph.size();
    if (n == 0) return {0, {{}, {}}};

    // Create a vector to store the nodes
    std::vector<int> nodes;
    for (const auto& pair : graph) {
        nodes.push_back(pair.first);
    }

    cout << "nodes : ";
    for (auto fi : nodes)
    {
        cout << fi << ", ";
    }
    cout << endl;
    // Initialize the minimum cut
    int min_cut = INT_MAX;
    std::vector<int> groupA, groupB;

    // Track groups dynamically
    std::vector<std::vector<int>> node_groups;
    for (int node : nodes) {
        node_groups.push_back({node});  //  {0, 1, 2, 3}
    }

    // Perform the algorithm
    while (nodes.size() > 1) {
        std::vector<int> used(nodes.size(), 0);   //{0,0,0,0} //[1,0,0,0] //[1,0,0,1]
        std::vector<int> weights(nodes.size(), 0);//{0,0,0,0} //[0, 1, -5, 5] //[0, -2, -7, 5]
                                                            //max_weight = 0 // max_weight = 5
        int prev = -1, last = 0;
        for (int i = 0; i < nodes.size(); ++i) {        // i = 2 3
            // Find the most strongly connected unused node
            cout << "~~~~~~~~~~~~~~~~~~~~~~~~process i " << i << endl;
            int max_weight = INT_MIN;
            for (int j = 0; j < nodes.size(); ++j) {
                if (!used[j] && weights[j] > max_weight) {
                    max_weight = weights[j]; //5
                    last = j; //3
                    cout << "j " << j << ", and max_weight " << max_weight << ", last " << last << endl;
                }
            }       // max_weight = 0, last = 0;

            // Mark the selected node as used
            used[last] = 1;     //[1,0,0,0] //[1,0,0,1]
            // if (weights[last] < min_cut)
            // {
            //     minimal_cut_group.clear();
            //     minimal_cut_group.insert(minimal_cut_group.end(), node_groups[last].begin(), node_groups[last].end());
            // }

            cout << "used : ";
            for (auto fi : used)
                cout << fi  << ", ";
            cout << endl;

            // If it's not the first node, update the cut
            if (i == nodes.size() - 1) {
                cout << "i = size -1, and mi_cut " << min_cut << endl;
                if (weights[last] < min_cut) {
                    min_cut = weights[last];

                    // Separate nodes into two groups
                    groupA.clear();
                    groupB.clear();

                    for (int k = 0; k < node_groups.size(); ++k)
                    {
                        if (k == last)
                            groupB.insert(groupB.end(), node_groups[k].begin(), node_groups[k].end());
                        else
                            groupA.insert(groupA.end(), node_groups[k].begin(), node_groups[k].end());
                    }

                    // for (int k = 0; k < nodes.size(); ++k) {
                    //     if (used[k]) {
                    //         // groupB.insert(groupB.end(), node_groups[k].begin(), node_groups[k].end());
                    //         groupB.push_back(nodes[k]);
                    //     } else {
                    //         // groupA.insert(groupA.end(), node_groups[k].begin(), node_groups[k].end());
                    //         groupA.push_back(nodes[k]);
                    //     }
                    // }

                    cout << "min_cut " << min_cut << endl;
                    cout << "groupA : ";
                    for (const auto fi : groupA)
                        cout << fi << ", ";
                    cout << endl;

                    cout << "groupB : ";
                    for (const auto fi : groupB)
                        cout << fi << ", ";
                    cout << endl;
                }

                // Create a copy of the graph to avoid unintended side effects
                Graph temp_graph = graph;

                // Merge last node into prev node
                cout << "merge the last node and prev node : " << last << ", " << prev << endl;
                for (int k = 0; k < nodes.size(); ++k) {
                    if (k != last && k != prev) {
                        temp_graph[nodes[prev]][nodes[k]] += graph[nodes[last]][nodes[k]];
                        temp_graph[nodes[k]][nodes[prev]] += graph[nodes[k]][nodes[last]];
                    }
                }

                // Update the original graph after all changes are calculated
                graph = temp_graph;
                cout << "updata graph : " << endl;
                for (const auto fi : graph)
                {
                    for (const auto si : fi.second)
                        cout << "[ " << fi.first << ", " << si.first << " ] " << si.second << endl; 
                }

                // Merge node groups
                node_groups[prev].insert(node_groups[prev].end(), node_groups[last].begin(), node_groups[last].end());
                node_groups.erase(node_groups.begin() + last);
                cout << "updata node_group" << endl;
                for (const auto fi : node_groups)
                {
                    cout << "vector : ";
                    for (const auto si : fi)
                        cout << si << ", ";
                    cout << endl;
                }

                // Remove the last node
                nodes.erase(nodes.begin() + last);
                cout << "updata nodes : " << endl;
                for (const auto fi : nodes)
                {
                    cout << fi << ", ";
                    cout << endl;
                }
                break;
            }

            prev = last; //0  3 0

            // Update weights of unused nodes
            for (int j = 0; j < nodes.size(); ++j) {
                if (!used[j]) {
                    weights[j] += graph[nodes[last]][nodes[j]];//[0, 1, -5, 5] //[0, -2, -7, 5] //[0, -1, -12, 5]
                }
            }
            cout << "updata weight : ";
            for (auto fi : weights)
                cout << fi << ", ";
            cout << endl;
        }
    }

    // cout << "minimal cut group : ";
    // for (const auto fi : minimal_cut_group)
    //     cout << fi << ", ";
    // cout << endl;

    return {min_cut, {groupA, groupB}};
}

int main() {
    // Define the graph
    Graph graph;
    // graph[0][1] = 1;
    // graph[0][2] = -5;
    // graph[0][3] = 5;

    // graph[1][0] = 1;
    // graph[1][2] = 4;
    // graph[1][3] = -3;

    // graph[2][0] = -5;
    // graph[2][1] = 4;
    // graph[2][3] = -2;

    // graph[3][0] = 5;
    // graph[3][1] = -3;
    // graph[3][2] = -2;


    // graph[0][1] = 4;
    // graph[0][2] = 3;
    // graph[0][3] = 5;

    // graph[1][0] = 4;
    // graph[1][2] = 1;
    // graph[1][3] = 1;

    // graph[2][0] = 3;
    // graph[2][1] = 1;
    // graph[2][3] = 2;

    // graph[3][0] = 5;
    // graph[3][1] = 1;
    // graph[3][2] = 2;


    graph[0][1] = 7;
    graph[0][2] = 9;
    graph[0][3] = -3;

    graph[1][0] = 7;
    graph[1][2] = 4;
    graph[1][3] = -2;

    graph[2][0] = 9;
    graph[2][1] = 4;
    graph[2][3] = 6;

    graph[3][0] = -3;
    graph[3][1] = -2;
    graph[3][2] = 6;

    // Find the minimum cut
    auto result = stoerWagner(graph);
    std::cout << "The minimum cut of the graph is: " << result.first << std::endl;
    std::cout << "Group A: ";
    for (int node : result.second.first) {
        std::cout << node << " ";
    }
    std::cout << std::endl;
    std::cout << "Group B: ";
    for (int node : result.second.second) {
        std::cout << node << " ";
    }
    std::cout << std::endl;

    return 0;
}

