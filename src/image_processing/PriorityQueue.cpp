#include "image_processing/PriorityQueue.hpp"

#include <limits>
#include <stdexcept>

PriorityQueue::PriorityQueue(int num_nodes)
    : queue_(), cost_(num_nodes, std::numeric_limits<Cost>::infinity()) {}

bool PriorityQueue::empty() {
    discard_stale_entries();
    return queue_.empty();
}

void PriorityQueue::decrease_key(Node node, Cost new_cost) {
    if (node < 0 || node >= static_cast<int>(cost_.size())) {
        throw std::out_of_range("PriorityQueue::decrease_key: node fora de alcance");
    }

    if (new_cost < cost_[node]) {
        cost_[node] = new_cost;
        queue_.emplace(new_cost, node);
    }
}

std::pair<PriorityQueue::Node, PriorityQueue::Cost> PriorityQueue::pop() {
    discard_stale_entries();
    if (queue_.empty()) {
        throw std::runtime_error("PriorityQueue::pop: fila vazia");
    }

    Entry top = queue_.top();
    queue_.pop();
    return {top.second, top.first};
}

int PriorityQueue::num_nodes() const {
    return static_cast<int>(cost_.size());
}

const std::vector<PriorityQueue::Cost>& PriorityQueue::costs() const {
    return cost_;
}

void PriorityQueue::discard_stale_entries() {
    while (!queue_.empty()) {
        const Entry& top = queue_.top();
        Node node = top.second;
        Cost cost = top.first;

        if (node < 0 || node >= static_cast<int>(cost_.size())) {
            queue_.pop();
            continue;
        }

        if (cost != cost_[node]) {
            queue_.pop();
            continue;
        }

        break;
    }
}
