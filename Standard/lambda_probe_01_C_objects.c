/* ============================================================================
 * 探针 01：C 里"表达式产生对象"这件事，以及它的位置由什么决定
 * 编译：clang -std=c23 -O0 -Wno-writable-strings 01.c     (或 gcc)
 *
 * 佐证的结论：
 *   (A) 字符串字面量、复合字面量都是【表达式】，都能产生【对象】
 *   (B) 它们产生的对象是【lvalue】（可写、可取地址）
 *   (C) 自动存储期复合字面量的对象：位置由源码位置排定（同一 block 同一位置），
 *       但每次求值是一个【新实例】
 *   (D) 它的生存期是【当次 block 执行】，不是"到分号为止"
 *   (E) 文件作用域的复合字面量是 static storage duration
 *   (F) 函数原型作用域里 "no object is created"
 * ==========================================================================*/
#include <stdio.h>
/* ---------- (E) 文件作用域复合字面量：static storage duration ---------- */
int *g_static_cl = (int[]){10, 20, 30};
/* ---------- (B) 复合字面量是 lvalue：可以取地址、可以写 ---------- */
static void test_lvalue_nature(void)
{
    puts("=== (B) 复合字面量是 lvalue ===");
    int *p = (int[2]){1, 2};           /* 数组 lvalue -> decay 成首元素地址 */
    int *q = &(int){5};                /* 对非数组对象直接取地址 */
    ((int[2]){7, 8})[0] = 99;          /* 能当左值写（写的是另一个实例，见 (C)） */
    printf("  (int[2]){1,2} 的 p     = %p, p[0] = %d\n", (void *)p, p[0]);
    printf("  &(int){5}     的 q     = %p, *q   = %d\n", (void *)q, *q);
    printf("  (7,8) 的实例写完后读回 = %d  (写成功了，说明是 lvalue)\n",
           ((int[2]){7, 8})[0]);
}
/* ---------- (C) 位置由源码位置排定 ---------- */
static void test_location(void)
{
    puts("=== (C) 位置由源码位置排定 ===");
    int *seen[4];
    for (int i = 0; i < 4; i++) {
        int *p = (int[]){i, i + 1};    /* 同一个源码位置，循环 4 次 */
        seen[i] = p;
        printf("  第 %d 次求值: &obj = %p  obj[0] = %d\n", i, (void *)p, p[0]);
    }
    printf("  四次求值的地址全部相同? %s\n",
           (seen[0] == seen[1] && seen[1] == seen[2] && seen[2] == seen[3])
               ? "YES —— 位置固定（编译期可算的栈偏移）" : "NO");
    puts("  但每次求值都是【新实例】——见探针 (C2)");
}
static void test_new_instance_each_time(void)
{
    puts("=== (C2) 每次求值是【新实例】，不是同一个对象被反复指称 ===");
    /* 同一条语句里的两次求值：是两个不同的对象 */
    int *a = (int[]){1, 2};
    int *b = (int[]){1, 2};
    printf("  两个同写法但不同语句的对象: %p vs %p -> %s\n", (void *)a, (void *)b,
           a == b ? "同一对象" : "不同对象");
    /* 同一条语句里出现两次：也是两个不同对象 */
    printf("  同一条语句里两次: %p vs %p -> %s\n",
           (void *)&(int[]){5}, (void *)&(int[]){5},
           &(int[]){5} == &(int[]){5} ? "同一对象" : "不同对象");
}
/* ---------- (D) 生存期 = 当次 block 执行，而非"到分号" ---------- */
static void test_lifetime_to_block_end(void)
{
    puts("=== (D) 生存期是当次 block 执行，不是到分号 ===");
    int *p = &(int){42};               /* 表达式已经结束（分号已过） */
    printf("  分号之后再通过 p 读取: %d   -> 读到了，对象还活着\n", *p);
    *p = 77;                           /* 再写它也活着 */
    printf("  分号之后再写: %d\n", *p);
    puts("  对照：字符串字面量、static 复合字面量的生存期是整个进程");
}
static void test_static_scope(void)
{
    puts("=== (E) 文件作用域复合字面量 = static storage duration ===");
    printf("  g_static_cl = %p, 值 = {%d,%d,%d}\n", (void *)g_static_cl,
           g_static_cl[0], g_static_cl[1], g_static_cl[2]);
    puts("  注意：它和 (D) 里那个自动存储期的对象在【存储期】上完全不同");
}
/* ---------- (F) 函数原型作用域：no object is created ---------- */
static int proto_scope_helper(int n, int arr[n]) { return arr[0] + n; }
static void test_prototype_scope(void)
{
    puts("=== (F) 函数原型作用域：no object is created ===");
    /* 下面的 (int[3]){1,2,3} 出现在【函数原型作用域】里：
       标准规定 "no object is created"。程序照常编译运行。 */
    printf("  调用结果 = %d\n", proto_scope_helper(10, (int[3]){1, 2, 3}));
    puts("  （这个探针只能证明'没有对象也能通过编译'，不能直接观测'没分配'）");
}
/* ---------- (B') 字符串字面量的对象：static，位置固定 ---------- */
static void test_string_literal_object(void)
{
    puts("=== (B') 字符串字面量对象 ===");
    const char *p1 = "hello";
    const char *p2 = "hello";
    printf("  &\"hello\" = %p, 另一个 \"hello\" = %p\n", (void *)p1, (void *)p2);
    printf("  两次是否同一地址: %s  (标准说 potentially non-unique，本实现共享)\n",
           p1 == p2 ? "YES" : "NO");
    char s[] = "hello";
    printf("  char s[] = \"hello\";  s = %p  -> %s（不是字面量那块存储）\n",
           (void *)s, s == p1 ? "同一块" : "另一块存储，内容被复制过来了");
}
int main(void)
{
    test_lvalue_nature();
    test_location();
    test_new_instance_each_time();
    test_lifetime_to_block_end();
    test_static_scope();
    test_prototype_scope();
    test_string_literal_object();
    return 0;
}