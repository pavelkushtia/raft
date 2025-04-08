#include "skip_list.h"
#include <algorithm>

namespace sstable {

SkipList::SkipList()
    : head_(std::make_shared<Node>("", "", kMaxLevel)),
      max_level_(1),
      rng_(std::random_device{}()),
      dist_(0.0, 1.0) {
    for (int i = 0; i <= kMaxLevel; ++i) {
        head_->forward[i] = nullptr;
    }
}

int SkipList::RandomLevel() {
    int level = 1;
    while (dist_(rng_) < kProbability && level < kMaxLevel) {
        ++level;
    }
    return level;
}

std::shared_ptr<SkipList::Node> SkipList::FindGreaterOrEqual(
    const std::string& key,
    std::vector<std::shared_ptr<Node>>* prev) const {
    if (prev) {
        prev->resize(kMaxLevel + 1);
        std::fill(prev->begin(), prev->end(), head_);
    }

    auto current = head_;
    for (int i = max_level_; i >= 0; --i) {
        while (current->forward[i] && current->forward[i]->key < key) {
            current = current->forward[i];
        }
        if (prev) {
            (*prev)[i] = current;
        }
    }
    return current->forward[0];
}

bool SkipList::Insert(const std::string& key, const std::string& value) {
    std::vector<std::shared_ptr<Node>> prev(kMaxLevel + 1);
    auto node = FindGreaterOrEqual(key, &prev);

    if (node && node->key == key) {
        node->value = value;
        return true;
    }

    int level = RandomLevel();
    if (level > max_level_) {
        for (int i = max_level_ + 1; i <= level; ++i) {
            prev[i] = head_;
        }
        max_level_ = level;
    }

    auto new_node = std::make_shared<Node>(key, value, level);
    for (int i = 0; i <= level; ++i) {
        new_node->forward[i] = prev[i]->forward[i];
        prev[i]->forward[i] = new_node;
    }

    return true;
}

bool SkipList::Get(const std::string& key, std::string* value) const {
    auto node = FindGreaterOrEqual(key, nullptr);
    if (node && node->key == key) {
        *value = node->value;
        return true;
    }
    return false;
}

bool SkipList::Delete(const std::string& key) {
    std::vector<std::shared_ptr<Node>> prev(kMaxLevel + 1);
    auto node = FindGreaterOrEqual(key, &prev);

    if (!node || node->key != key) {
        return false;
    }

    for (int i = 0; i <= max_level_; ++i) {
        if (prev[i]->forward[i] != node) {
            break;
        }
        prev[i]->forward[i] = node->forward[i];
    }

    while (max_level_ > 0 && !head_->forward[max_level_]) {
        --max_level_;
    }

    return true;
}

std::vector<std::pair<std::string, std::string>> SkipList::GetAllEntries() const {
    std::vector<std::pair<std::string, std::string>> entries;
    auto node = head_->forward[0];
    while (node) {
        entries.emplace_back(node->key, node->value);
        node = node->forward[0];
    }
    return entries;
}

} // namespace sstable 