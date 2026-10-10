# Prim算法笔记

## 最小生成树(MST,Minimum Cost Spanning Tree)

对于一个带权的连通无向图,最小生成树是指包含图中所有顶点、边的数量=顶点数-1(边正好能把所有顶点连起来且不多出任何一条边,如果去除任意一条边都破坏该图的连通性,同时说明其无环连通)、所有边的权值之和最小的一个图(也可以解释成无序树)的结构

MST是最优子结构,因为整张图的MST必由MST的子图构成

## 邻接矩阵

邻接矩阵是表示图结构的一种常用方式

对于一个有n个节点的图，可以用一个n*n的矩阵来表示节点间的连接关系:

带权图:矩阵中第i行第j列的值表示从节点i到节点j的边的权重。若两点间没有直接边,通常用0,∞或一个特殊值如null表示,设计的算法负责识别不导通的特征

非带权图:可用布尔值1/0表示连接与否,1代表连通,0代表不连通

如果图是无向图,则邻接矩阵是对称矩阵,即第i行第j列的值等于第j行第i列的值表示节点i与j之间的边是双向且权重相同的,用表示有向图的邻接矩阵以等价性表示无向图

基于STL的邻接矩阵实现:

```
class AdjacencyMatrix
{
public:
    int nodenumber,edgenumber;
    vector<char> node;
    vector<vector<int>> matrix;

    AdjacencyMatrix(int nodes):nodenumber(nodes),edgenumber(0),matrix(nodes,vector<int>(nodes,0))
    {
        node.resize(nodes);//预分配,否则AddNode出错
    }
    void AddNode()
    {
        for(int i=0;i<nodenumber;++i)
        {
            char temp;
            cin>>temp;
            node[i]=temp;
        }
    }
    void AddEdge(int from,int to,int weight)
    {
        martix[from][to]=weight;
        ++edgenumber;
    }//这里可以选择直接cin叫用户输入全部,但是下面会写,所以这里写一个传参,有向图手动输入有向情况
};
```

手搓:

```
struct AMGraph
{
    char vexs[233];
    int arcs[233][233];
    int vexnum,arcnum;
};
void CreateNetwork(AMGraph &G);
int LocateVex(AMGraph &G,char vex);
int CreateN(AMGraph &G)
{
    cin>>G.vexnum>>G.arcnum;//输入顶点数边数
    for(int i=0;i<G.vexnum;++i)
    {
        cin>>G.vexs[i];
    }
    for(int i=0;i<G.vexnum;++i)
    {
        for(int j=0;j<G.vexnum;++j)
        {
            G.arcs[i][j]=0;
        }
    }
    for(int k=0;k<G.arcnum;++k)
    {
        char v1,v2;
        int weight;
        cin>>v1>>v2>>weight;
        int i=LocateVex(G,v1);
        int j=LocateVex(G,v2);
        G.arcs[i][j]=weight;
    }
}
int LocateVex(AMGraph &G,char vex)
{
    for(int i=0;i<G.vexnum;++i)
    {
        if(G.vexs[i]==vex)
        {
            return i;
        }
    }
    return -1;
}
```

## Prim算法(基于手搓)

Prim算法用于寻找连通图的最小生成树,它将图分为两部分:一部分是正在构建的不完整MST,另一部分是尚未连接的节点,算法的核心任务是:每一步都通过"选择连接这两部分的最小权重边",将一个新的节点归并入MST(这确保归并后的部分树始终满足最小生成树的局部性质,即虽然不包括图的所有节点,但为最终MST的子图,满足去除一条边破坏连通性和权值之和最小性质),此过程反复进行,直至MST覆盖整个图的所有节点

Prim算法用到一个数组closeedge,这个数组存储的是**不完整的MST之外的点**的信息,刻画它们到不完整的MST内一点的最小权值边,数组索引对应图内所有点的编号,该点若在MST内则adjvex可为任意值(因为算法只考察且归并:不在不完整的MST内的节点,在MST内的节点不会被用到,不考虑这个值的作用),lowcost=0,具体见下

```
struct closeedge
{
    char adjvex;//到MST内哪一点
    int lowcost;//权值为多少
};
```

Prim算法的过程:

1.将初始顶点加到不完整的MST中,对其余不在不完整的MST内的每一个顶点,将对应的closeedgearray元素,都初始化为到初始顶点的信息

也即adjvex都是初始顶点,lowcost为不在不完整的MST内的每一个顶点到初始顶点的距离

2.循环n-1次,作如下处理:

从closeeedgearray选出最小边(不为0,lowcost最小),并且对于该lowcost对应的点i,将i和其下的adjvex连接,输出此边(adjvex,i)

将i加入不完整的MST中

更新closeedgearray,具体来说,对于没加到不完整的MST中的边,在上面把一个点归并到MST后如果剩下的点到不完整的MST的点和最小权值有改变,则改变其

程序:

```
int LocateVex(AMGraph &G,char vex)
{
    for(int i=0;i<G.vexnum;++i)
    {
        if(G.vexs[i]==vex)
        {
            return i;
        }
    }
    return -1;
}
int Min(closeedgearray array[],AMGraph &G)
{
    int min=66666;
    int index=-1;
    for(int i=0;i<G.vexnum;++i)
    {
        if(array[i].lowcost<min&&array[i].lowcost!=0)
        {
            min=array[i].lowcost;
            index=i;
        }
    }
    return index;
}
void MiniSpanTree_Prim(AMGraph G,char u)
{
    int k=LocateVex(G,u);
    for(int j=0;j<G.vexnum;++j)
    {
        if(j!=k)
        {
            closeedgearray[j]={u,G.arcs[k][j]};
        }
    }
    closeedgearray[k].lowcost=0;
    for(int i=1;i<G.vexnum;++i)
    {
        k=Min(closeedge);//表示含有最短到不完整的MST的边的顶点
        char u0=closeedgearray[k].adjvex;
        char v0=G.vexs[k];
        cout<<u0<<v0;
        closeedgearray[k].lowcost=0;
        for(int j=0;j<G.vexnum;++j)//j表示没加到不完整的MST的点
        {
            if(G.arcs[k][j]<closeedgearray[j].lowcost)
            {
                closeedgearray[j]={G.vexs[k],G.arcs[k][j]};
            }
        }
    }
}
```

对于for(int j=0;j<G.vexnum;++j){if(G.arcs[k][j]<closeedgearray[j].lowcost){closeedgearray[j]={G.vexs[k],G.arcs[k][j]};}}的讨论:

为什么只需要考察新加入的顶点k?关键原因:MST的性质:当顶点k加入MST后,对于任意未加入顶点j,它到新MST的最小距离只可能是以下两种情况之一:

原有的最优连接:在k加入前,j已经通过某条边连接到旧MST,这条边的权重记录在closeedgearray[j].lowcost中,对应的连接点是closeedgearray[j].adjvex;

通过新顶点k连接,从j到新加入的顶点k有一条边,这条边的权重是G.arcs[k][j]

所以不需要考察其他可能性,因为除开这个for考察的,其他都是最优的

二部图视角:Prim算法维护两个集合:已加入MST的顶点集合和未加入的顶点集合。每个未加入的顶点记录着它到已加入集合的最小边权重以及对应的已加入顶点。当从未加入集合中选择一个离已加入集合最近的顶点加入MST后,不用检查已加入集合中其他顶点与未加入顶点之间的边,因为每个未加入顶点已经保存了到已加入集合的最小权重,如果已加入集合中其他顶点能提供更小的权重,那么在该顶点加入时就已经被记录了,因此,每次更新只需要考察新加入的顶点

其他讨论:

Prim算法是贪心算法,因为它每次循环都选择最小边

Prim算法(基于邻接矩阵)一般用于边比点多的图,因为画图可以看出,Prim算法逐步增加不完整的MST的顶点数,可称为加点法,由于算法内会查找不存在的边(相对于输入算法的G的完全图)(具体来说是),所以如果不存在的边多于存在的边,查找的无效次数增多。这具体体现在Min函数的for循环每次都要检查n个顶点,但对于不存在的遍多于存在的边的情况,lowcost更新的几率降低,但算法仍需要逐个检查这些顶点,但没有任何有效信息;同时,更新closeedgearray循环同样检查所有顶点到新顶点的距离,但对于不存在的遍多于存在的边的情况,剩下的点到新MST点更多是无边连接,所以查找无效的几率提高

Prim算法还可以基于优先队列,直接把closeedgearray元素装到优先队列即可,在min处由小根堆pop出,再修改下面的更新逻辑,这可以改善上面所说的无效查找的情况,这里略
