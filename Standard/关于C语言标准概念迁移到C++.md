我在补充学习java的函数式接口可以用lambda就地override时,顺带想了解lambda表达式在cpp中的地位

回顾c标准里表达式的相关概念:17个表达式,可被evaluate

至今我接触的相关表达式,求值之后都是基础类型的值,比如a=b假设二者都是int,那么b左值转换的结果就是int,比如1或者2或者3;再者就是数组发生decay

这个点很重要,因为它表征了rvalue一般都是基础类型可以有的值,我们不需要一个对象(这里不是OO层面的意义)的Address,Alignment,Storage Duration就可以表征它们

**使用这个c抽象机框架理论解释cpp遇到了困难**,因为当我们`MyClass Instance1=Instance2`时,等号右边的结果明显不是基础类型,它直观来看是一个真的对象,它可以有Address,有Alignment,有Storage Duration(至于前面的左值转换的结果都是基础类型,那结果可以理解为立即数之类的)

我们也便无法用普通的对一个expression进行evaluate,之后确定side effect这个过程来表征`MyClass Instance1=Instance2`等过程

如何从现有知识去暂时解释或者类比?我有两个方向

第一个是课上老生常谈的**拷贝语义**,第二个是类比`char a[]="hello"`和`char* a="hello"`也即c标准的string literal分配对象

为什么能联想到这两者?因为拷贝语义真的是老生常谈的,每个cpp学习者在大学课上已经听到耳朵都起茧的;string literal能联想也同样基于拷贝语义,因为抽象机行为下,`char a[]="hello"`是在只读的.rodata定义一个string literal,再通过memcpy复制到a对象里,这里同样和拷贝语义有一个复制过程

那么拷贝语义底层是怎么运作的?这里从上面提到的`MyClass Instance1=Instance2`展开,在每节大学的cpp课上都会提到,我们写一个复制构造函数在类内,有些场景会用到它

``` cpp
class Person
{
    string name;
    int age;
    Person(const string &n,int a):name(n),age(a)
    {
        //普通构造函数
    }
    Person(const Person &other):name(other.name),age(other.age)
    {
        //复制构造函数
    }
    Person &operator=(const Person &other)
    {
        if(this!=&other)
        {
            name=other.name;
            age=other.age;
        }
        return *this;
    }
};
```

实际就是函数调用而已

**但是**,cpp的对象初始化方式多种多样,`MyClass Instance1=Instance2`触发复制构造函数,那么

``` cpp
int a=1;
int b=1;
auto MyLambda=[a,b](int c,int d)
{
    return a+b+c+d;
};
MyLambda(1,1);
```

这个行为里,我们有一个具名对象MyLambda,有一个就地定义的lambda表达式,这里底层真的触发了复制构造函数吗?

答案是否,参阅https://eel.is/c++draft/dcl.init.general

```
(15.6.1)
If the initializer expression is a prvalue and the cv-unqualified version of the source type is the same as the destination type, the initializer expression is used to initialize the destination object.
[Example 2: T x = T(T(T())); value-initializes x ([basic.lval], [expr.type.conv]). — end example]
(15.6.2)
Otherwise, if the initialization is direct-initialization, or if it is copy-initialization where the cv-unqualified version of the source type is the same as or is derived from the class of the destination type, constructors are considered. The applicable constructors are enumerated ([over.match.ctor]), and the best one is chosen through overload resolution ([over.match]). Then:
```

后略

关键:initializer表达式(等号右边的表达式)是纯右值(prvalue,cpp有右值引用语义,所以出了个将亡值,属于右值的一种,我暂且不知,网上说纯右值就是c标准里的右值),且原类型的cv未限定版本(非const,非volatile,这句话就是我们c标准老师课上超爱讲的非原子非限定类型里的子集)与目标类型相同,则使用该初始化器表达式来初始化目标对象

也即满足(15.6.1)条件,左边对象会直接指向等号右边表达式分配出来的对象(这句话也稍不严谨,实际上`auto Mylambda=lambda体`这句话的内涵是一个对象,零个关系,我这句话说的反而成了两个对象,一个关系了,能理解所谓**一个对象,零个关系**即可)

这里有个很奇妙的地方,auto本身就是根据右值类型(表达式求值后的非原子非限定类型)自动推断对象类型的(c标准课上有讲),所以`auto MyLambda=lambda体`这句话很明确是走(15.6.1)逻辑去初始化的

再来看(15.6.2):否则,如果该初始化是直接初始化,或者它是复制初始化,且源类型的cv未限定版本与目标类相同或是目标类的派生类,则考虑构造函数...(后略)

直接初始化:声明对象时不用=符号,而是把构造参数放圆括号或者花括号,让编译器直接调用匹配的构造函数

复制初始化:用=从另一个表达式初始化对象...**C++17起,某些情况下保证拷贝省略,但语法上仍属于复制初始化**

ref:https://en.cppreference.com/index.php?title=cpp/language/copy_elision

```
Since C++17, a prvalue is not materialized until needed, and then it is constructed directly into the storage of its final destination. This sometimes means that even when the language syntax visually suggests a copy/move (e.g. copy initialization), no copy/move is performed...
```

所以我们之前的(15.6.1),就是拷贝省略的标准里的具体原文体现

所以我们如何去解释这个等于号的过程?答案至此明晰:(15.6.1)的拷贝省略,或者(15.6.2)的调用复制构造函数的初始化

我们始终在讨论的是对象与对象在initializer和等于号下的行为

第二,`char a[]="hello"`和`char* a="hello"`如何结合着去理解?

答案也很清楚,前者是通过string literal分配对象规则在.rodata分配对象,之后memcpy复制过来;后者是直接把指针指向.rodata(虽然使用该指针操作往往UB),行为上,前者和复制构造有相似之处,后者和拷贝省略有相似之处

---

我们还有一个点没有明晰,(15.6.1)里说initializer表达式是纯右值,如何理解?按照c标准,所有表达式不都是要经过左值转换或者decay才得到右值吗?我们这里lambda表达式属于表达式家族一员,但是`MyClass Instance1=Instance2`里Instance2也是表达式家族一员,为什么我们不说Instance2是纯右值走(15.6.1)?

参阅:https://eel.is/c++draft/basic.lval

```
6 Whenever a glvalue appears as an operand of an operator that requires a prvalue for that operand, the lvalue-to-rvalue ([conv.lval]), array-to-pointer ([conv.array]), or function-to-pointer ([conv.func]) standard conversions are applied to convert the expression to a prvalue.
```

每当一个glvalue作为某个运算符的操作数出现,而该运算符要求该操作数为prvalue时,就会应用lvalue-to-rvalue、array-to-pointer或function-to-pointer标准转换,将该表达式转换为prvalue

glvalue是广义左值,普通左值和将亡值的合称,由于我们这里依旧不讨论右值引用,因此这句话可窄化(对glvalue与prvalue概念在没有右值引用情况下再定义,重定义)为

每当一个左值作为某个运算符的操作数出现,而该运算符要求该操作数为右值时,就会应用左值转换,退化或function-to-pointer标准转换,将该表达式转换为右值

在我们的两个例子`auto MyLambda=Lambda体`与`MyClass Instance1=Instance2`里,这二者**都是初始化,不是表达式**,`a=b`才是表达式而`int a=b`不是

可能有初学者会问:`int a=b`这不有个等号吗为什么不是表达式

因为标准就是分开的,这玩意叫initializer初始化器,和声明declaration合在一起是去定义和初始化一个对象,就像c标准里我们不把`int a=b`里的a叫变量,我们叫它对象标识符object identifier,因为比如`volatile const int`它既能被修改(MMIO场景)又不能被修改(编码者角度),那我们究竟是把它叫变量还是常量(我们老师的标准提案就是要彻底消灭variable在标准里指称object的字眼),事实证明标准上和初学还是有很大裂缝

因此,**这里没有任何运算符要求操作数被转换为右值**,所以也就没有Instance2被转为纯右值走(15.6.1)

继续回看15.6,补充一下我上面的问题:为什么我们不说Instance2是纯右值走(15.6.1)

这句话应该改为:为什么我们不说lambda表达式不是右值,所以避开了(15.6.1)?因为c标准里右值都要通过表达式求值获得

cpp标准的一句话杀死比赛:https://eel.is/c++draft/expr.prim.lambda

```
2 A lambda-expression is a prvalue whose result object is called the closure object.
```

我就说lambda表达式是右值你能把我怎么滴。。。所以它无疑是走(15.6.1)了

需要在cpp破除的观念:右值必须由表达式求出

---

我们还有一个点没明晰:我们始终在讨论的是对象与对象在initializer和等于号下的行为

那么,我们知道`a=b`发生了左值转换,side effect确定,我们上面也知道了所谓的讨论中心的行为无外乎两个,也即拷贝省略和复制初始化

我们也知道了复制初始化就是调用复制构造函数,

那拷贝省略的底层呢?

为什么`auto MyLambda=lambda体`在标准里定义lambda体是prvalue纯右值?

为什么它叫表达式还能被定义为纯右值?

为什么要这么设计?

现在先回顾一下c里定义的表达式:17种,我在笔记里写着,n3096.pdf也明摆着(6.5 Expressions)

再回顾一下c里分配对象4种方式:初始化器initializer,字符串字面量string literal,复合字面量compound literal,内存管理函数malloc等

查阅n3096.pdf,可以发现有

```
6.5.1 Primary expressions
Syntax
primary-expression:
identifier
constant
string-literal
( expression )
generic-selection
```

(这就是老师上课也经常喜欢问的基础表达式有哪几个:标识符、常量、字符串字面量、括号括起来的表达式、泛型选择)

以及

```
6.5.2 Postfix operators
Syntax
1
postfix-expression:
primary-expression
postfix-expression [ expression ]
postfix-expression ( argument-expression-listopt )
postfix-expression . identifier
postfix-expression-> identifier
postfix-expression ++
postfix-expression-
compound-literal
```

可以看到,字符串字面量和复合字面量都是表达式,它们能分配对象,我的笔记里(也是出自老师的课件)

``` markdown
## lvalue表达式

1.对象标识符

2.数组下标运算表达式

3.*exp

4.String Literal(字符串字面量)

5.Compound Literal

6.指向结构体/联合体的lvalue.member

7.指向结构体/联合体指针表达式->member

注意:string literal是lvalue表达式,定位char[]对象,不是常量
```

分配对象的表达式都是左值,`(Student *)malloc(sizeof(MyClass));`整体没有左值,但是它的结果是右值,这一坨本身肯定就不是右值

至此我们得到的信息有:c里,能分配对象的表达式都是左值表达式;cpp里,能分配对象的表达式可能是右值(比如标准里指定lambda是prvalue)

我之前的课程和笔记从来没有讨论过表达式和分配对象之间的关系,我能获取的核心只有:c里,左值表达式定位一个对象,求值后得出右值,它可以被理解为立即数;表达式可以二分为左值表达式和非左值表达式,比如加加减减,它不定位,但是依靠其他右值来返回一个右值

如果要基于目前对于c的表达式与分配对象之间的关系去讨论为什么分配对象必须是左值表达式,让我们重申c标准里对于右值的定义:value of expression,它是一个value,它没有address,没有alignment,没有storage duration,而分配对象恰好意味着产生一个具备这三样(甚至更多属性)的事物,所以rvalue分配对象这个表达在c的世界里就是自相矛盾

~~这也指出了,c里,当我们对这个表达式(可以分配对象的左值表达式)求值,它才产生出对象~~

**↑这句话是错的**那么具体来说是对左值表达式求值之后才分配对象吗?这可不是,下面会提到,file scope里编译期分配,block scope里进block时分配,这个表达式只是一个类似于@Component的玩意,告诉编译器请提前在什么时候分配我,就像注解告诉容器在你启动时请把我造成bean一样(spring乱入hhh)

我们反反复复说分配对象的方式是什么字符串字面量,复合字面量容易以为我们执行了它才分配,事实不是如此的,但是它(c抽象机)在所谓的先有对象后有指称它的左值这个逻辑上确实自洽

以上是**为什么c里分配对象的表达式一定是左值表达式**的论述

至于为什么cpp里分配对象的表达式可以是右值:

cpp里的体系与c里完全不同,它是直接规定了表达式属于哪些值类别,再每一类各自规定求值做什么

ref:https://eel.is/c++draft/basic.lval 开篇点题

```
(1.1)
A glvalue is an expression whose evaluation determines the identity of an object, function, non-static data member, or a direct base class relationship.
(1.2)
A prvalue is an expression whose evaluation initializes an object or computes the value of an operand of an operator, as specified by the context in which it appears, or an expression that has type cv void.
(1.3)
An xvalue is a glvalue that denotes an object whose resources can be reused (usually because it is near the end of its lifetime).
(1.4)
An lvalue is a glvalue that is not an xvalue.
(1.5)
An rvalue is a prvalue or an xvalue.
```

(1.2)一针见血指出了纯右值是一个表达式,它的evaluation会初始化一个对象,或者...

至此问题:为什么`auto MyLambda=lambda体`在标准里定义lambda体是prvalue纯右值?

答案是:标准里定义纯右值的求值可能会初始化一个对象,lambda体初始化一个对象,因此lambda体是纯右值

接下来的两个问题同根同源,为什么就要把它定义为纯右值,而不是左值,让其求值后再产生对象?

**因为左值要定位一个对象,而lambda体这个对象,如果将其定义为左值,那它指称的对象根本不存在**

---

很抽象的一个问题是,cpp意识到了左值可能指向一个不存在的对象这个严重问题,所以定义上直接将其定义为右值

那c设计不就存在严重缺陷了吗?这是最后一个问题

我从对象的生命周期和存储周期来解决

我的原笔记里指出,string literal的存储周期是静态的,编译时决定;复合字面量在block里是automatic的,在其他地方(file scope)可以static

c标准原文(n3096.pdf 97页)指出一个复合字面量:

```
if the storage duration is automatic, the lifetime of the instance of the unnamed object is the current execution of the enclosing block
```

这句话重点关注the current execution of the enclosing block,也即,当我们没有说出复合字面量这句话时,这个block事实已经存在它了,就跟我们没说string literal时,编译期它已经进入了未来的.rodata区域里去了差不多一个道理

因此,**抽象机执行逻辑在左值表达式没被执行之前,该对象事实已经存在**,对于那些static的更不用说

所以,c里左值可能指向一个不存在的对象这个问题不存在的,抽象机和编译器发力了

但是cpp里我们`auto MyLambda=lambda体`这句话在运行时执行时才分配一个lambda对象,因此lambda体就不能是一个左值

至此我们的问题完全解决

---

一些小讨论:

1.为什么cpp会这样设计,那我编译器设计成也是就像java扫标签一样扫描一下哪里有lambda,然后block进入时初始化不就可以避免这种重新定义底层的繁琐行为了吗

我感觉可能是因为cpp实现的更极致性能,就像它还因为什么怕复制开销太大,所以引入右值引用这个机制(虽然我到现在还是懒得学右值引用等机制,我只是道听途说和背八股文,知道右值引用大概解决这个问题);可以预见的是,cpp面向对象,一个对象可以有多个成员,如果你在lambda里尝试捕获了太多对象,这些对象自己又很大,加之lambda对象本身是一个闭包对象,它包含了那些被捕获的对象(捕获的变量会成为闭包对象的数据成员),那么直接进入block都是一个不小的开销了

不过可以说明的是,不和c一样block进入初始化而在执行到语句才初始化,有cpp自己的理由,性能就是很好的一个佐证,这个讨论比之前的简单多了

2.cpp最初始并不是真的分成左值,将亡值,纯右值,然后左值和将亡值合成泛左值,将亡值和纯右值合称右值;早期cpp没有出现右值引用这种机制时确实还是c传承下来的一套,<https://stroustrup.com/terminology.pdf>里Bjarne Stroustrup教授自己开篇在Background节都说了

```
The terms “lvalue” and “rvalue” are deep in C++’s genes... I did the same for early definitions of C++ (e.g. see [Stroustrup,1986] and [Ellis,1989]). The terms “lvalue” and “rvalue” are “all over” the draft C++ standard. Clearly, this is not terminology to mess with without serious reason and great care. So – despite appearances and rumors to the contrary – we didn’t.
```

之后还是这篇文章,他在Naming一节直接指出

```
I note that this whole discussion/problem arise from the introduction of rvalue references and move semantics. These notions simply don’t exist in Strachey’s world consisting of just rvalues and lvalues. Someone observed that the ideas that

    • Every value is either an lvalue or an rvalue

    • An lvalue is not an rvalue and an rvalue is not an lvalue

are deeply embedded in our consciousness, very useful properties, and traces of this dichotomy can be found all over the draft standard.
```

注意关键词:rvalue references、move semantics,究其根本,是右值引用和移动语义把左值右值的传统二分导向了左值、将亡值、纯右值体系,这篇文章里提到的啥iM、im、Im大概就是这三者的雏形(虽然我也不咋看得懂)

当然我们上面讨论的lambda这种闭包对象就地初始化而不是进块后初始化,也是cpp不得不背弃传统的一个体现

(不是根因,虽然lambda和右值引用、移动语义都在C++11被引入;但是强制拷贝省略是C++17才被正式引入的,如果不看右值引用和移动语义,那需要一个破除二分的新体系不是因为lambda,是因为拷贝省略,也就是上面所谓的一个对象零个关系的表现,与性能的根本要求,导致它不能进块初始化;拷贝省略晚于移动语义和右值引用出现,所以拷贝省略出现就等于直接吃上了这份框架红利)

3.强制拷贝省略带来一个重要影响:该类不再需要拥有可访问的复制构造函数,相关程序也能成功编译

4.我还顺便注意到一个问题,lambda它要捕获啊,如果出现了一个VLA运行时确定长度那你在进入block捕获一个没确定长度(实际根本不存在)的VLA老弟你不炸了吗,上面说了捕获对象是闭包对象的成员,你这VLA连存储都没分配怎么进闭包对象,这是lambda不能进block就造的又一个理由

---

我叫ai写了一些可以佐证上面讨论结果的程序,详见

lambda_probe_01_C_objects.c

lambda_probe_02_cpp_value_category.cpp

lambda_probe_03_elision.cpp

lambda_probe_04_elision_switch.cpp

lambda_probe_05_closure_storage.cpp