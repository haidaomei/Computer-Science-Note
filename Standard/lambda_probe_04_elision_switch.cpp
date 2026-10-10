/* ============================================================================
 * 探针 04：可选省略可以被关掉，保证的那部分关不掉
 *
 * 编译并对比：
 *   clang++ -std=c++20 -O0 04.cpp                          -> 只看到 1 次构造
 *   clang++ -std=c++20 -O0 -fno-elide-constructors 04.cpp  -> 多出拷贝构造
 *   （对 T obj = T(); 这种，任何开关都无法让它多出一次拷贝构造）
 *
 * 佐证的结论：
 *   (A) NRVO（return 具名对象）是"可选省略"：可以关掉
 *   (B) 15.6.1（同类型 prvalue 初始化）是语义规定：关不掉
 *   (C) 所以这两者不是同一种机制
 * ==========================================================================*/
#include <cstdio>
static int g_copy = 0;
struct C {
    int v = 0;
    C() { std::puts("    C()         普通构造"); }
    C(const C &o) : v(o.v) { ++g_copy; std::puts("    C(const C&) 拷贝构造"); }
    C(C &&o) : v(o.v) { ++g_copy; std::puts("    C(C&&)      移动构造"); }
    ~C() { std::puts("    ~C()        析构"); }
};
/* (A) 可选省略：return 具名对象 */
static C named_return()
{
    C local;
    return local;                  /* NRVO 候选 */
}
/* (B) 保证的那一类：return prvalue */
static C prvalue_return()
{
    return C{};                    /* 15.6.1，直接初始化 result object */
}
int main()
{
    std::puts("--- 1) C a = named_return();  (可选省略) ---");
    C a = named_return();
    std::puts("--- 2) C b = prvalue_return(); (保证) ---");
    C b = prvalue_return();
    std::puts("--- 3) C c = C();             (保证) ---");
    C c = C();
    std::printf("总拷贝/移动构造次数 = %d\n", g_copy);
    std::puts("对比两次运行的这个数字：");
    std::puts("  默认         : 只有可选省略那部分会变");
    std::puts("  -fno-elide-constructors: 数字变大，且第 3 项仍然不变");
    (void)a; (void)b; (void)c;
    return 0;
}