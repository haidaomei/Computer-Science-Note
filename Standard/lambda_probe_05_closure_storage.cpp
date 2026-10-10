/* ============================================================================
 * 探针 05：闭包对象的存储与生存期
 *          ——捕获的数据只存一份；对象的位置由"谁接住它"决定
 *
 * 编译：clang++ -std=c++20 -O0 05.cpp
 *
 * 佐证的结论：
 *   (A) 闭包对象的大小近乎等于捕获成员之和（Closure = 数据成员）
 *   (B) 捕获数据只有【一份存储】，不是"一份在别处 + 一份在闭包里"
 *   (C) 同一份源码位置，每次求值产生【不同对象】
 *   (D) 生存期由"谁接住它"决定：
 *         auto f = []{...}          -> 活到 block 结束
 *         take([]{...})             -> 形参对象，函数返回即析构
 *         []{...}()                 -> 物化出的临时对象，完整表达式结束即析构
 * ==========================================================================*/
#include <cstdio>
#include <cstddef>
struct Big {
    unsigned char pad[64];
};
struct Tracked {
    int id;
    explicit Tracked(int i) : id(i) { std::printf("      Tracked(%d) 构造\n", id); }
    Tracked(const Tracked &o) : id(o.id) { std::printf("      Tracked(%d) 拷贝构造\n", id); }
    ~Tracked() { std::printf("      Tracked(%d) 析构\n", id); }
};
template <class F> void take_by_value(F f) { (void)f; std::puts("      [take_by_value 体内]"); }
struct ProbeOffsets {
    unsigned char nothing;
    Big big;
};
int main()
{
    std::puts("=== (A)(B) 闭包 = 数据成员，捕获数据只有一份 ===");
    std::printf("  sizeof(Big)                                 = %zu\n", sizeof(Big));
    std::printf("  offsetof(ProbeOffsets,big)                  = %zu\n", offsetof(ProbeOffsets, big));
    Big big_buf{};
    auto no_cap = [] { return 0; };
    auto cap    = [big_buf] { return big_buf.pad[0]; };
    std::printf("  sizeof([]{...})   无捕获                     = %zu  (空类，至少 1)\n", sizeof(no_cap));
    std::printf("  sizeof([big_buf]{...}) 捕获一个 64 字节对象  = %zu\n", sizeof(cap));
    std::printf("  差值 = %zu  -> 闭包对象里【只有】那份捕获的数据，没有别的影子\n",
                sizeof(cap) - sizeof(no_cap));
    std::puts("\n=== (C) 同一份源码位置，每次求值都是新对象 ===");
    auto f1 = [] { return 0; };
    auto f2 = [] { return 0; };
    /* 注意：这两个地址类型不同，直接 == 会被编译器拒绝（见编译报错），
       这本身就是"每个 lambda 表达式类型唯一"的证据。这里退化成 void* 比较。 */
    std::printf("  &f1 = %p\n  &f2 = %p   -> %s\n", (void *)&f1, (void *)&f2,
                (void *)&f1 == (void *)&f2 ? "同一对象" : "不同对象（且类型也不同）");
    std::puts("\n=== (D) 生存期由'谁接住它'决定 ===");
    std::puts("  -- D1: auto f = [...];  活在 block 内 --");
    {
        auto f = [t = Tracked{1}] { (void)t; };
        std::puts("     构造完闭包，下面离开内层 block");
    }
    std::puts("     <- 这里才析构（block 结束），不是分号");
    std::puts("  -- D2: take_by_value([...])  形参对象 --");
    take_by_value([t = Tracked{2}] { (void)t; });
    std::puts("     <- 函数返回就析构");
    std::puts("  -- D3: [...]()  丢弃值表达式，物化出临时对象 --");
    [t = Tracked{3}] { (void)t; }();
    std::puts("     <- 完整表达式结束即析构");
    return 0;
}