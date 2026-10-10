/* ============================================================================
 * 探针 03：保证的初始化 vs 可选的拷贝省略
 *          ——同一个源位置，-std=c++14 与 -std=c++17 结论相反
 *
 * 编译并对比：
 *   clang++ -std=c++14 -O0 03.cpp   -> 编译失败
 *   clang++ -std=c++17 -O0 03.cpp   -> 编译通过
 *   clang++ -std=c++20 -O0 03.cpp   -> 编译通过
 *   再试：clang++ -std=c++20 -O0 -fno-elide-constructors 04.cpp（见探针 04）
 *
 * 佐证的结论：
 *   (A) 15.6.1 是【保证】的：构造函数压根不在候选里，删掉也能编译
 *   (B) 可选省略（NRVO、return 具名对象）是【允许】的：构造函数必须可访问
 *   (C) 因此"拷贝省略"这个词下藏着两种性质完全不同的东西
 * ==========================================================================*/
#include <cstdio>
#include <utility>
struct Box {
    int v = 7;
    Box() = default;
    Box(const Box &) = delete;
    Box(Box &&) = delete;
};
/* ---------- (A) 保证的那一类 ---------- */
/* A1: 同类型 prvalue 直接初始化（[dcl.init.general]/15.6.1） */
static void a1(void)
{
    Box b = Box();                 /* C++17 起：不调任何构造函数 */
    std::printf("  A1 Box b = Box();            OK, b.v = %d\n", b.v);
}
/* A2: 返回 prvalue */
static auto a2_make() { return [b = Box{}] { return b.v; }; }
static void a2(void)
{
    auto l = a2_make();            /* result object 就是 l */
    std::printf("  A2 return [b=Box{}]{...}     OK, l() = %d\n", l());
}
/* A3: 用 prvalue 初始化同类型对象（lambda 的直接形态） */
static void a3(void)
{
    auto l = [b = Box{}] { return b.v; };
    std::printf("  A3 auto l = [b=Box{}]{...}   OK, l() = %d\n", l());
}
/* ---------- (B) 可选的那一类 ---------- */
/* B1: return 具名对象 -> NRVO（可选）。构造函数必须可访问，哪怕编译器不会调。 */
#if defined(TRY_OPTIONAL)
static auto b1_make()
{
    auto l = [b = Box{}] { return b.v; };
    return std::move(l);           /* 这里会要求移动构造可访问 -> 报错 */
}
static void b1(void)
{
    auto l = b1_make();
    std::printf("  B1 return std::move(具名)     OK, l() = %d\n", l());
}
#endif
int main()
{
    a1();
    a2();
    a3();
#if defined(TRY_OPTIONAL)
    b1();
#endif
    return 0;
}