# Dijkstra算法笔记

## 图的基本概念

1、图(Graph):由节点(Vertex)、边(Edge)组成

2、有向图:边有方向的图

3、无向图:边没有方向的图

4、权重(Weight):边的属性,表示边长,可以不完全理解为节点到节点的位移;权重可以为负值,该情况下可能会派生负权环

## 路径相关概念

1、路径(Path):一系列边连接的节点序列，如 A->B->C

2、最短路径:从某节点到某节点权重最小的路径(不一定是边数最少、经过节点最少)

3、环(Cycle):起点和终点相同的路径

4、负权环:环的总权重为负值,会导致无限绕圈降低总权重

5、单源最短路径(Single-Source):求从一个起点到图中所有其他节点的最短路径

6、多源最短路径(All-Pairs):求所有节点之间的最短路径

## 图的表示方法

这里使用嵌套容器vector<vector<pair<int, int>>>表示一个图,这个嵌套容器代表一个二维数组,每个数组元素是一个pair,每个pair中有两个数字元素,分别代表邻接节点和权重,二维数组的行标表示对应节点,即二维数组在每一行存储特定节点的数据,这种情况下节点是被有序表示的(从0开始到最后一个节点)

比如一个节点为0、1、2、3的图,其中0分别与1、2、3都相邻;1、2、3同理,这代表一个节点分别为0、1、2、3的四边形,现定义0到1权重1,1到2权重2,2到3权重3,3到0权重4,0到2权重5,1到3权重6

(这里建议草稿画图表示)

另外,图的表示还分为有向图和无向图,下面会给出上面的图写成有向图和无向图的示例

使用该嵌套容器存储有向图的定义在c++中如下:

    vector<vector<pair<int, int>>> a(947);//初始化947行,每行的列没有初始化,后面会使用pushback往后推
    a[0].push_back({ 1, 1 });//0到1权重1
    a[0].push_back({ 2, 5 });//0到2权重5
    a[1].push_back({ 2, 2 });//1到2权重2
    a[1].push_back({ 3, 6 });//1到3权重6
    a[2].push_back({ 3, 3 });//2到3权重3
    a[3].push_back({ 0, 4 });//3到0权重4

使用该嵌套容器存储无向图的定义在c++中如下:

```
graph[0].push_back({1, 1});
graph[1].push_back({0, 1});
graph[0].push_back({2, 5});
graph[2].push_back({0, 5});
graph[1].push_back({2, 2});
graph[2].push_back({1, 2});
graph[1].push_back({3, 6});
graph[3].push_back({1, 6});
graph[2].push_back({3, 3});
graph[3].push_back({2, 3});
graph[3].push_back({0, 4});
graph[0].push_back({3, 4});
```

经过对比可以发现,有向图使用数组行号代表点,使用pair中的first代表与其相邻的点,数组行号和pair中的first组成的关系即为一个箭头,从数组行号代表的点指向与其相邻的点;无向图中,来回指向就代表了无向

为什么来回指向代表无向?这是基于Dijkstra算法得出的,Dijkstra算法本身处理的是有向图,比如求原点到某点的距离;但是如果来回指向一次,那么Dijkstra算法又会算一次某点到原点的距离,而且这个某点到原点的距离不一定和刚开始有向的原点到某点的距离相同,当学完了Dijkstra算法,可以把以上这两个样例放到程序中,观察它们结果的不同,然后在草稿上结合现在的解释理解

为什么要尽量初始化多一些行?为求方便

特别地,这种图的表示法称为邻接表(Adjacency List)

## Dijkstra算法(基于邻接表)

Dijkstra算法只能使用在无负权的图中

Dijkstra算法只能使用在单源最短路径问题中

Dijkstra算法的目标是在带权有向图或无向图中,找到一个起点(源点)到其他所有节点的最短路径,这个节点是分析对象

继续以上面的节点为0、1、2、3的图为例,现在创造一个一维数组distance,记录起点到每个节点的当前已知最短距离

起点到自己的距离为0,其他节点初始化为947,表示暂时不可达,也表示距离无限远(在其他题目可不能初始化为947,只能说尽量大,c++里面提供了一个模板类表示各种数据类型的极值,这里略)

于是初始化distance数组如下:

    distance[4]={0,947,947,947};

分别代表0到0的距离,0到1的距离,0到2的距离,0到3的距离

因为这里分析的是点0,所以这里将distance的第一位初始化为0,如果要分析1到其他点的距离则有初始化如下

    distance[4]={947,0,947,947}

接下来,选择0起点分析

分析0，遍历它的所有邻居(1和2和3),更新它们的距离:

    distance[0]=min(0,0)=0;
    distance[1]=min(1,947)=1;
    distance[2]=min(5,947)=5;
    distance[3]=min(4,947)=4;

接下来,选择未分析的distance中的最小距离节点,这一步的意思是选择距离起点最近的节点,比如这里选择的是点1

分析1,遍历它的所有邻居但是不遍历已经分析过的点(遍历2和3);更新起点,到被该步遍历的点,的距离

0->1->2有1+2=3,distance[2]=min(5,3)=3;更新

0->1->3有1+6=7,distance[3]=min(7,4)=4;不更新

接下来,选择未分析的distance中的最小距离节点,这一步的意思是选择距离起点最近的节点,比如这里选择的是2(注意,这里选择的点与该点到上一个选择的点的距离无关)

分析2,遍历它的所有邻居但是不遍历已经分析过的点(遍历3);更新起点，到被该步遍历的点,的距离

0->1->2->3有1+2+3=6,distance[3]=min(6,4)=4,不更新

于是得到了0到1、2、3的最小距离分别为1,3,4

以上就是具体的大概的步骤,总结起来就是:

1、使用邻接表初始化被分析的图

2、选择起点,初始化distance数组

3、遍历起点的邻居,更新被遍历的邻居到起点的距离

4、选择离起点最近的未分析的点,遍历该点未分析的邻居,更新该点被遍历的邻居到起点的距离

5、选择离起点最近的未分析的点,遍历该点未分析的邻居,更新该点被遍历的邻居到起点的距离

6、选择离起点最近的未分析的点,遍历该点未分析的邻居,更新该点被遍历的邻居到起点的距离

所以Dijkstra算法的核心就是456步

给出程序如下

```
vector<vector<pair<int, int>>> graph;//全局声明该图为graph,定义可以在main函数定义
vector<int> dijkstra(int start, int n)//dijkstra算法函数,要求输入一个图的起始点和一个图的节点数,输不输入节点数可以动态决定分析到哪些点
{
    vector<int> distance(n, INT_MAX);//创建一个含有n个元素,每个元素的值为关键字INT_MAX的数组distance,INT_MAX关键字代表int型数据所能储存的最大值
    vector<bool> visited(n, false);//布尔数组,表示每个节点是否已经处理,处理则1
    distance[start] = 0;//起点到自身的距离为0,注意这里必须为start不能为0,因为要处理的不一定是点0,而有可能是其他的点,这个点通过函数的形参输入
    for (int i = 0; i < n; i++)//循环n次,每次处理一个节点
    {
        int point1 = -1;//这里point1代表被处理的点,每次大循环都要初始化point1为-1,是代表还没选中一个点进行处理,选中之后会在下面这个for循环对point1进行赋值(如果点没被处理完,下面这个for循环是一定能找出未处理的点的,所以如果下面这个for循环没把point1赋值,说明点已经处理完毕,于是有下面这个for循环再下面的if判断)
        int minimumdistance = INT_MAX;//初始化未处理的点到原点的距离为INT_MAX(无限大)
        for (int j = 0; j < n; j++)//注意逻辑,这个for循环的作用是找到未处理的点,是先找到未处理的点,再处理找到的点,所以这个for循环要放在下面那个for循环的上面,也即先找再处理
        {
            if (!visited[j] && distance[j] < minimumdistance)//如果这个点未被visit,而且这个点到原点的distance小于minimumdistance,那么把最小距离设成这个点到原点的distance,并且选定这个点为j
            {
                minimumdistance = distance[j];
                point1 = j;
            }
        }//这个for循环是把所有的点都给遍历(j<n),正好对应了上面所说的这个for循环一定能找出未处理的点
        if (point1 == -1)//所有可达节点已处理完毕,结束
        {
            break;
        }
        visited[point1] = true;//标记为已处理,这个标记和上面的if判断的顺序没有严格要求,只是如果这个放在if的上面,如果所有节点都处理完毕后,会把最后一个已经处理,已经visit的point1给重新赋一次true
        for (auto analysis : graph[point1])//这个for的作用是遍历point1的所有邻居,更新距离,注意graph是一个二维数组,每个元素是一个pair
        {//这个for循环中analysis并不代表vector<pair<int,int>>,而是代表pair<int,int>;for (auto analysis : graph[point1])这种语法,在C++11之后,本身代表遍历冒号的顺序容器的每一个元素,所以这里是遍历顺序容器vector中的每一个pair,在进行程序的语言移植时需要特别注意
            int point2 = analysis.first;//这个point2表示的是被遍历的点
            int weight = analysis.second;//表示权重,也就是距离
            if (!visited[point2] && distance[point2] > distance[point1] + weight)//如果point2没被visit,且point2目前到原点的最短路径大于point1目前到原点的最短路径加point1到point2的距离
            {
                distance[point2] = distance[point1] + weight;//那么point2目前到原点的最短路径等于point1目前到原点的最短路径加上point1到point2的距离
            }//这个if的逻辑可以在草稿上演示一下,point1目前到原点的最短路径加上point1到point2的距离就是被更新的被遍历的点到原点的最短路径,这个最短路径取代了之前获得的被遍历的点到原点的最短路径
        }
    }
    return distance;
}
```

注意,这里被遍历不代表被分析,即使被遍历不被分析,那么在分析下一个点时,同样的点可能会再被遍历

这个算法还可以被简单地优化成输入特定的二维数组,输入该二维数组代表的图的起点,输入要分析的点数,声明大概是vector<int> dijkstra(vector<vector<pair<int, int>>> graph, int start, int n)

MATLAB移植版,图使用以下测试样例隐含的表示方法给出:

```
function distance=dijkstra(graph,start,n)
distance=zeros(1,n);
for a=1:n
    distance(a)=Inf;
end
distance(start)=0;
visited=zeros(1,n);
for b=1:n
    point1=-1;
    minimumdistance=Inf;
    for c=1:n
        if ~visited(c)&&distance(c)<minimumdistance
            minimumdistance=distance(c);
            point1=c;
        end
    end
    if point1==-1
        break;
    end
    visited(point1)=1;
    for analysis=graph(point1,:)
        matrix=analysis{1};
        point2=matrix(1);
        weight=matrix(2);
        if ~visited(point2)&&distance(point2)>distance(point1)+weight
            distance(point2)=distance(point1)+weight;
        end
    end
end
```

测试样例:使用二维元胞数组,每个元胞为一个包含两个数字的行向量

```
graph={[1,1],[2,5];[2,2],[3,6];[3,3],[0,Inf];[0,4],[0,Inf]}
graph={[2,1],[3,5],[4,4];[1,1],[3,2],[4,6];[1,5],[2,2],[4,3];[1,4],[2,6],[3,3]}
```
