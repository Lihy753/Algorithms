Tarjan's Algorithm is used for finding all strongly connected components in graph.
      0
      |
      1
     / \
    2---3
     \ /
      4
      |
      5


| edge id | u | v |
| ------- | - | - |
| 0       | 0 | 1 |
| 1       | 1 | 2 |
| 2       | 2 | 3 |
| 3       | 3 | 1 |
| 4       | 2 | 4 |
| 5       | 4 | 3 |
| 6       | 4 | 5 |



桥：(0,1)、(4,5)

割点：1、4

EBCC：

{0}

{1,2,3,4}

{5}

block graph：{0} - {1,2,3,4} - {5}