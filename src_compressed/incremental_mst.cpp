 /* https://codeforces.com/blog/entry/158509 */ const int inf = 1e9; struct BottleneckTree { 
std::vector<int> link, cost; /* all inserted costs must be < inf */ BottleneckTree(int n) : 
link(n, 0), cost(n, inf) { link[0] = -1; cost[0] = inf + 1; } void rot(int x) { int p = link[x], 
g = link[p]; swap(cost[x], cost[p]); link[x] = g, link[p] = x; } void skip(int x) { link[x] = 
link[link[x]]; } void access(int x) { for (int p = link[x]; p != -1; p = link[x]) { int g = 
link[p]; if (cost[x] <= cost[p]) rot(x); /* (A) */ else if (cost[p] > cost[g]) skip(p), skip(x); 
/* (B) */ else if (cost[g] <= cost[x]) rot(p), skip(x); /* (C) */ else skip(x); /* (D) */ } } 
int Query(int x, int y) { /* x != y; inf if disconnected */ access(x); access(y); return 
cost[x]; } int AddEdge(int x, int y, int c) { /* x != y */ access(x); access(y); if (cost[x] > 
c) swap(cost[x], c); return c; } };  
