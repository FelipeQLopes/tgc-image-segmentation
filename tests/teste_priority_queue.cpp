#include "PriorityQueue.hpp"

#include <cassert>
#include <iostream>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)                                                             \
    do {                                                                       \
        std::cout << "  [RUN ] " << (name) << std::endl;                       \
    } while (0)

#define PASS(name)                                                             \
    do {                                                                       \
        std::cout << "  [PASS] " << (name) << std::endl;                       \
        ++tests_passed;                                                        \
    } while (0)

static void test_initially_empty() {
    const char* name = "fila vazia ao criar";
    TEST(name);

    PriorityQueue pq(5);
    assert(pq.empty());
    PASS(name);
}

static void test_decrease_key_and_pop_order() {
    const char* name = "decrease_key e ordem de extração";
    TEST(name);

    PriorityQueue pq(4);
    pq.decrease_key(0, 10.0);
    pq.decrease_key(1, 4.5);
    pq.decrease_key(2, 7.3);
    pq.decrease_key(3, 4.4);

    auto [node1, cost1] = pq.pop();
    assert(node1 == 3 && cost1 == 4.4);

    auto [node2, cost2] = pq.pop();
    assert(node2 == 1 && cost2 == 4.5);

    auto [node3, cost3] = pq.pop();
    assert(node3 == 2 && cost3 == 7.3);

    auto [node4, cost4] = pq.pop();
    assert(node4 == 0 && cost4 == 10.0);

    PASS(name);
}

static void test_lazy_decrease_key_ignores_obsolete_entries() {
    const char* name = "lazy decrease_key ignora entradas obsoletas";
    TEST(name);

    PriorityQueue pq(3);
    pq.decrease_key(0, 50.0);
    pq.decrease_key(1, 30.0);
    pq.decrease_key(0, 20.0);
    pq.decrease_key(1, 25.0);

    auto [node1, cost1] = pq.pop();
    assert(node1 == 0 && cost1 == 20.0);

    auto [node2, cost2] = pq.pop();
    assert(node2 == 1 && cost2 == 25.0);

    PASS(name);
}

static void test_invalid_node_throws() {
    const char* name = "decrease_key com node invalido";
    TEST(name);

    PriorityQueue pq(2);
    bool threw = false;
    try {
        pq.decrease_key(5, 1.0);
    } catch (const std::out_of_range&) {
        threw = true;
    }

    assert(threw);
    PASS(name);
}

int main() {
    test_initially_empty();
    test_decrease_key_and_pop_order();
    test_lazy_decrease_key_ignores_obsolete_entries();
    test_invalid_node_throws();

    std::cout << "\nTests passed: " << tests_passed << "\n";
    return tests_failed == 0 ? 0 : 1;
}
