/* ============================================================================
 * 探针 02：C++ 的 prvalue / result object / 物化 / 初始化分派 / lambda
 * 编译：clang++ -std=c++20 -O0 02.cpp
 *       （探针 7 需要单独用 -std=c++14 对比）
 *
 * 佐证的结论：
 *   (1) auto f = []{};  只有一个对象，没有临时对象
 *   (2) 每个 lambda 表达式类型唯一；每次求值都是新对象
 *   (3) 物化：绑引用/当丢弃值时才产生临时对象
 *   (4) 15.6.1 vs 15.6.2：源是 prvalue 还是 lvalue 决定一切
 *   (5) 保证初始化 vs 可选省略：构造函数"不在候选里" vs "必须可访问"
 * ==========================================================================*/
#include <cstdio>
#include <utility>
/* ---------- 可追踪的捕获对象：看构造/析构顺序 ---------- */
struct Tracked {
    int id;
    explicit Tracked(int i) : id(i) { std::printf("    Tracked(%d) 构造\n", id); }
    Tracked(const Tracked& o) : id(o.id) { std::printf("    Tracked(%d) 拷贝构造\n", id); }
    ~Tracked() { std::printf("    Tracked(%d) 析构\n", id); }
};
template <class F> void take_by_value(F f) { (void)f; }
/* ---------- 探针 1 ---------- */
static void probe1_one_object(void)
{
    std::puts("=== (1) auto f = []{}; 只有一个对象 ===");
    auto f = [] { return 1; };
    std::printf("  &f            = %p   (闭包对象的地址)\n", (void *)&f);
    /* 同一个源码位置的 lambda，写成两条语句 -> 两个不同对象 */
    auto g = [] { return 1; };
    std::printf("  &g            = %p   (另一个 lambda 表达式 -> 另一个对象)\n", (void *)&g);
    std::printf("  sizeof(f)     = %zu\n", sizeof(f));
    std::printf("  sizeof([]{})  = %zu  (prvalue 出现在 sizeof 里：不求值、不物化)\n",
                sizeof([] { return 1; }));
    /* 真正的证据：数构造/析构次数。
       如果 15.6.1 之外还造了一个临时对象再拷进来，
       这里就会多出一次"拷贝构造 + 析构"。
       对照 probe4：源是 lvalue 时确实会多出一次拷贝构造。 */
    std::puts("  -- 计数证据：下面这段只应出现 1 次构造、1 次析构 --");
    {
        auto ff = [t = Tracked{100}] { (void)t; };
        std::puts("  -- 到这里为止：如果还出现过 'Tracked(100) 拷贝构造'，");
        std::puts("     说明 15.6.1 之外还造了临时对象（实际并没有） --");
        (void)ff;
    }
}
/* ---------- 探针 2 ---------- */
static void probe2_type_uniqueness(void)
{
    std::puts("=== (2) 每个 lambda 表达式类型唯一 ===");
    auto l1 = [] {};
    auto l2 = [] {};
    std::printf("  l1/l2 类型相同? %d   (0 = 不同，即使源码长得一样)\n",
                (int)__is_same(decltype(l1), decltype(l2)));
    auto l3 = l1;                       /* 同一个表达式的对象 -> 可以拷贝 */
    std::printf("  l1/l3 类型相同? %d   (1 = 同类型，因为 l3 拷贝自 l1)\n",
                (int)__is_same(decltype(l1), decltype(l3)));
}
/* ---------- 探针 3 ---------- */
static void probe3_materialization(void)
{
    std::puts("=== (3) 物化：只在需要 glvalue 时才产生临时对象 ===");
    const auto &r = [] { return 7; };   /* 绑定引用 -> 必须物化 */
    std::printf("  &r = %p   (物化出来的临时闭包对象，生存期被引用延长)\n", (void *)&r);
    std::printf("  r() = %d\n", r());
    /* 丢弃值表达式 -> 也会物化 */
    [] { std::puts("    (丢弃值表达式里的 lambda 被调用了)"); }();
    /* 用 Tracked 看物化的生存期 */
    std::puts("  -- 临时对象的生存期 --");
    auto named = [t = Tracked{1}] {};   /* result object 就是 named，活到 block 结束 */
    take_by_value([t = Tracked{2}] {}); /* 形参对象，函数返回即析构 */
    std::puts("  -- named 还活着，形参的那个已经析构 --");
    (void)named;
}
/* ---------- 探针 4 ---------- */
static void probe4_init_dispatch(void)
{
    std::puts("=== (4) 15.6.1 vs 15.6.2：源是 prvalue 还是 lvalue ===");
    /* lvalue 源（具名 lambda 对象）-> 15.6.2 -> 真的调拷贝构造 */
    auto src = [t = Tracked{3}] {};
    std::puts("  auto dst = src;   // src 是 lvalue");
    auto dst = src;                     /* 这里会打印一次 Tracked 拷贝构造 */
    std::puts("  -- 上面那次就是 15.6.2：拷贝构造函数被调用 --");
    /* prvalue 源 -> 15.6.1 -> 不经过任何构造函数 */
    std::puts("  auto dst2 = []{...};  // 源是 prvalue");
    auto dst2 = [t = Tracked{4}] {};    /* 只打印一次 Tracked 构造，没有拷贝 */
    std::puts("  -- 上面只有一次构造，没有拷贝构造：15.6.1 --");
    (void)dst; (void)dst2;
}
/* ---------- 探针 5：15.6.1 的"不调构造函数"硬证据 ---------- */
struct Nomove {
    int v = 5;
    Nomove() = default;
    Nomove(const Nomove &) = delete;
    Nomove(Nomove &&) = delete;
};
static void probe5_no_ctor_needed(void)
{
    std::puts("=== (5) 15.6.1：拷贝/移动构造被 delete 也能编译（C++17 起）===");
    /* 这是探针 7 里用 -std=c++14 编译会失败的那一行 */
    Nomove n = Nomove();
    auto lam = [b = Nomove{}] { return b.v; };
    std::printf("  n.v = %d, lam() = %d   -> 编译通过，说明构造函数不在候选里\n",
                n.v, lam());
}
int main()
{
    probe1_one_object();
    probe2_type_uniqueness();
    probe3_materialization();
    probe4_init_dispatch();
    probe5_no_ctor_needed();
    return 0;
}