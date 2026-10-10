# KMP算法笔记

## BF算法

BF算法是枚举算法,待匹配的子串(下称模式串)从第一个字符开始和主串向后匹配,如果遇到一个不同,就前进一位(模式串首和主串第二个字符),和被查找匹配的串(下称主串)继续比较,程序如下

手搓,1-based:

```
#define MAXLEN 255
typedef struct
{
    char ch[MAXLEN+1];//出自人邮社数据结构第二版,这里+1不是存储末尾0,而是这本书让index从1开始,不同于0-based索引,下面的算法也是
    int length;
}SString;
int Index_BF(SString S,SString T,int pos)//pos是搜索开始的索引,该程序语境一般为1
{
    int i=pos;//模式串的首位匹配主串的位置
    int j=1;//模式串的索引,用于和主串匹配每一位
    while(i<=S.length&&j<=T.length)
    {
        if(S.ch[i]==T.ch[j])
        {
            ++i;
            ++j;
        }
        else
        {
            i=i-j+2;
            j=1;
        }
    }
    if(j>T.length)
    {
        return i-T.length;
    }
    else
    {
        return 0;
    }
}
```

基于STL,0-based:

```
int Index_BF(string text,string pattern,int pos)
{
    //这里可以添加pos参数检查,不允许小于0大于主串长
    int i=pos;
    int j=0;
    while(i<text.length()&&j<pattern.length())
    {
        if(text[i]==pattern[j])
        {
            ++i;
            ++j;
        }
        else
        {
            i=i-j+1;
            j=0;
        }
    }
    if(j==pattern.length())//不是大于
    {
        return i-pattern.length();
    }
    else
    {
        return -1;
    }
}
```

## KMP算法(0-based,人邮社数据结构第二版为1-based)

### 概念

前缀:从字符串开头开始的连续子串,不包含整个字符串本身,也即不包含最后一个字符

前缀集合:由一个字符串的所有前缀构成的集合(顺序)

例:"helloworld"的前缀集合:{"h","he","hel","hell","hello","hellow","hellowo","hellowor","helloworl"}

后缀:以字符串末尾结束的连续子串,不包含整个字符串本身,也即不包含第一个字符

后缀集合:由一个字符串的所有后缀构成的集合(顺序)

例:"helloworld"的后缀集合:{"d","ld","rld","orld","world","oworld","loworld","lloworld","elloworld"}

注意:"helloworld"的后缀集合不是{reverse("d"),reverse("ld"),reverse("rld"),reverse("orld"),reverse("world"),reverse("oworld"),reverse("loworld"),reverse("lloworld"),reverse("elloworld")},这就是上面说的顺序的意思

公共前后缀:对于一个字符串,既是前缀又是后缀的子串,这从前后缀集合里面找

最长公共前后缀:在一个字符串中,既是前缀又是后缀的最长子串(有时候也会把它当成该子串的长度)

### next数组(next[0]=-1)

next数组记录了**每个位置之前的子串中,有多长的前缀和后缀是相同的**

对于每个位置,把它看作一个独立字符串,记录这个字符串的最长公共前后缀,其值为对应位next数组的值

例:helloworld的next数组

***

第一位(next[0]):h

该字符串的前缀集合:∅

该字符串的后缀集合:∅

该字符串的最长公共前后缀:∅

该字符串的最长公共前后缀长度:0

***

第二位(next[1]):he

该字符串的前缀集合:{"h"}

该字符串的后缀集合:{"e"}

该字符串的最长公共前后缀:∅

该字符串的最长公共前后缀长度:0

***

第三位(next[2]):hel

该字符串的前缀集合:{"h","he"}

该字符串的后缀集合:{"l","el"}

该字符串的最长公共前后缀:∅

该字符串的最长公共前后缀长度:0

***

第四位(next[3]):hell

该字符串的前缀集合:{"h","he","hel"}

该字符串的后缀集合:{"l","el","ell"}

该字符串的最长公共前后缀:∅

该字符串的最长公共前后缀长度:0

***

第五位(next[4]):hello

该字符串的前缀集合:{"h","he","hel","hell"}

该字符串的后缀集合:{"o","lo","llo","ello"}

该字符串的最长公共前后缀:∅

该字符串的最长公共前后缀长度:0

***

第六位(next[5]):hellow

该字符串的前缀集合:{"h","he","hel","hell","hello"}

该字符串的后缀集合:{"w","ow","low","llow","ellow"}

该字符串的最长公共前后缀:∅

该字符串的最长公共前后缀长度:0

***

第七位(next[6]):hellowo

该字符串的前缀集合:{"h","he","hel","hell","hello","hellow"}

该字符串的后缀集合:{"o","wo","owo","lowo","llowo","ellowo"}

该字符串的最长公共前后缀:∅

该字符串的最长公共前后缀长度:0

***

第八位(next[7]):hellowor

该字符串的前缀集合:{"h","he","hel","hell","hello","hellow","hellowo"}

该字符串的后缀集合:{"r","or","wor","owor","lowor","llowor","ellowor"}

该字符串的最长公共前后缀:∅

该字符串的最长公共前后缀长度:0

***

第九位(next[8]):helloworl

该字符串的前缀集合:{"h","he","hel","hell","hello","hellow","hellowo","hellowor"}

该字符串的后缀集合:{"l","rl","orl","worl","oworl","loworl","lloworl","elloworl"}

该字符串的最长公共前后缀:∅

该字符串的最长公共前后缀长度:0

***

第十位(next[9]):helloworld

该字符串的前缀集合:{"h","he","hel","hell","hello","hellow","hellowo","hellowor","helloworl"}

该字符串的后缀集合:{"d","ld","rld","orld","world","oworld","loworld","lloworld","elloworld"}

该字符串的最长公共前后缀:∅

该字符串的最长公共前后缀长度:0

***

| 位置(next[i]) | 子串 | 前缀集合 | 后缀集合 | 最长公共前后缀 | 长度 |
|-|-|-|-|-|-|
| next[0] | h | ∅ | ∅ | ∅ | 0 |
| next[1] | he | {"h"} | {"e"} | ∅ | 0 |
| next[2] | hel | {"h","he"} | {"l","el"} | ∅ | 0 |
| next[3] | hell | {"h","he","hel"} | {"l","el","ell"} | ∅ | 0 |
| next[4] | hello | {"h","he","hel","hell"} | {"o","lo","llo","ello"} | ∅ | 0 |
| next[5] | hellow | {"h","he","hel","hell","hello"} | {"w","ow","low","llow","ellow"} | ∅ | 0 |
| next[6] | hellowo | {"h","he","hel","hell","hello","hellow"} | {"o","wo","owo","lowo","llowo","ellowo"} | ∅ | 0 |
| next[7] | hellowor | {"h","he","hel","hell","hello","hellow","hellowo"} | {"r","or","wor","owor","lowor","llowor","ellowor"} | ∅ | 0 |
| next[8] | helloworl | {"h","he","hel","hell","hello","hellow","hellowo","hellowor"} | {"l","rl","orl","worl","oworl","loworl","lloworl","elloworl"} | ∅ | 0 |
| next[9] | helloworld | {"h","he","hel","hell","hello","hellow","hellowo","hellowor","helloworl"} | {"d","ld","rld","orld","world","oworld","loworld","lloworld","elloworld"} | ∅ | 0 |

程序:

```
//获取前缀集合(不包含空字符串,对于第一位next[0]=0会在下面的求next函数处理,这里不关心)
unordered_set<string> getPrefixes(const string &s)
{
    unordered_set<string> prefixes;
    for (int i = 1; i < s.length(); ++i)
    {
        prefixes.insert(s.substr(0, i));//substr是string类的一个成员函数,用于从串中提取子串,两个参数:起始索引和长度,第一个不是起始位置,第二个也不是终止索引
    }
    return prefixes;//如果i=length,那么假设
}
//获取后缀集合
unordered_set<string> getSuffixes(const string &s)
{
    unordered_set<string> suffixes;
    for (int i = 1; i < s.length(); ++i)
    {
        suffixes.insert(s.substr(s.length() - i, i));
    }
    return suffixes;
}
//获取两个集合的交集中最长的字符串长度
int longestCommonLength(const unordered_set<string> &set1, const unordered_set<string> &set2)
{
    int maxLength = 0;
    for (const auto &str : set1)//对于set1的每个串
    {
        if (set2.find(str) != set2.end())//如果在set2中找到同样的串
        {
            if (str.length() > maxLength)//如果这个串的长度大于之前的maxlength
            {
                maxLength = str.length();//那么把maxlength设成这个串长度
            }
        }
    }
    return maxLength;//如果找不到那就是0
}
//使用集合列举法计算next数组
vector<int> calculateNextBruteForce(const string &pattern)
{
    int n = pattern.length();
    vector<int> next(n, 0);

    //next[0] = 0,想换成next[0]=-1自己添加逻辑
    next[0] = 0;
    
    //对每个位置i,计算前i+1个字符组成的子串的最长公共前后缀长度
    for (int i = 1; i < n; ++i)
    {
        //获取前i+1个字符组成的子串
        string currentSubstr = pattern.substr(0, i + 1);

        //获取前缀集合和后缀集合
        unordered_set<string> prefixes = getPrefixes(currentSubstr);
        unordered_set<string> suffixes = getSuffixes(currentSubstr);
        
        //计算最长公共长度(排除整个字符串本身)
        int commonLength = longestCommonLength(prefixes, suffixes);
        
        next[i] = commonLength;
    }
    
    return next;
}
```

上面是把next[0]=0,直接体现了每个串的最长公共前后缀的长度,把该数组整体右移并覆盖最后一位,第一位赋-1后就得到了next[0]=-1的next数组,以上程序需要$n^3$的时间复杂度

1-based串的next数组统一在0-based串的数组值上+1,同时将该next数组整体移动到初始索引从1开始,0位空置无意义(由next[0]=0或next[0]=-1得到都可以,这得到1-based串的两种next数组标准);1-based串的next数组也区分右移与否,这只能发生在由next[0]=0统一+1得到的新next数组,此时不考虑index=0位,右移从index=1开始,index=1位补0(对应next[0]=-1后统一+1);人邮社数据结构第二版的next数组采用1-based串加右移

next数组还有线性时间复杂度的求解方法,这里请求补充

next数组右移和非右移的差别,这里请求补充

程序:

```
vector<int> compute_next(const string &pattern)
{
    int n = pattern.length();
    vector<int> next(n);
    
    //先计算next[0] = 0的版本
    next[0] = 0;
    for (int i = 1, j = 0; i < n; ++i)
    {
        while (j > 0 && pattern[i] != pattern[j])
        {
            j = next[j - 1];
        }
        if (pattern[i] == pattern[j])
        {
            j++;
        }
        next[i] = j;
    }
    
    //右移一位并设置next[0] = -1
    for (int i = n - 1; i > 0; --i)
    {
        next[i] = next[i - 1];
    }
    next[0] = -1;
    
    return next;
}
```

### 模式匹配(next[0]=-1,0-based)

在字符串匹配过程中,当模式串与主串进行逐字符比较时,若遇到不匹配的情况,传统方法(BF算法)需要将主串指针i回溯至本轮匹配起始位置的下一个字符，同时模式串指针j复位至模式串开头,这种操作称为主串指针回溯

KMP算法改进流程如下:

定位已匹配部分:当发生不匹配时,主串和模式串中已成功匹配的字符序列是完全相同的,这部分称为已匹配部分

提取公共前后缀:在已匹配部分中,寻找其自身的最长公共前后缀,即一个既是前缀又是后缀的子串,且长度尽可能大

对齐公共部分:由于已匹配部分在主串和模式串中一致,因此可以将模式串的前缀移动至与主串的后缀对齐,模式串的该前缀便覆盖了主串的对应后缀,两者重合

继续匹配:对齐后,模式串无需完全复位,仅需从公共前后缀的下一个字符开始,与主串当前指针位置继续比较,此时主串指针无需回溯,仍停留在原先不匹配的位置

程序:

```
int kmp_search(const string &text, const string &pattern, const vector<int> &next)
{
    int i = 0;//text指针
    int j = 0;//pattern指针
    
    while (i < text.length() && j < pattern.length())
    {
        if (j == -1 || text[i] == pattern[j])
        {
            //匹配成功或j回到起点，双指针后移
            i++;
            j++;
        }
        else
        {
            //失配时根据next数组跳转
            j = next[j];
        }
    }
    
    if (j == pattern.length())
    {
        return i - j; // 返回匹配起始位置
    }
    return -1; // 匹配失败
}
```

对于最长公共前后缀的"最长"的讨论:这里请求补充

