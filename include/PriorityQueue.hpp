#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

#include <queue>
#include <utility>
#include <vector>

class PriorityQueue {
public:
    using Node = int;
    using Cost = double;
    using Entry = std::pair<Cost, Node>;

    explicit PriorityQueue(int num_nodes);

    bool empty();
    void decrease_key(Node node, Cost new_cost);
    std::pair<Node, Cost> pop();

    int num_nodes() const;
    const std::vector<Cost>& costs() const;

private:
    struct Compare {
        bool operator()(const Entry& a, const Entry& b) const {
            return a.first > b.first;
        }
    };

    void discard_stale_entries();

    std::priority_queue<Entry, std::vector<Entry>, Compare> queue_;
    std::vector<Cost> cost_;
};

#endif // PRIORITY_QUEUE_HPP
