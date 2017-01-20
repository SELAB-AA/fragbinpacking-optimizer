#ifndef SOLUTION_H_
#define SOLUTION_H_

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

#include "item.h"

namespace optimizer {

class Problem;

/*
 * A `Solution` consists of a number of `Block`s with `Item`s.
 */
class Solution {
 public:
    /*
     * A Block is a set of items and a number of bins with equal capacity.
     * For each additional bin, a cut ensues, so the number of fragments
     * increases.
     */
    class Block {
        friend std::ostream &operator<<(std::ostream &os, const Block &block) {
            std::sort(block.begin_, block.end_,
                      [](const auto &l, const auto &r) { return l < r; });
            auto it = block.begin_;
            os << '(' << (*it)->size;
            for (++it; it != block.end_; ++it) {
                os << ", " << (*it)->size;
            }
            os << ')';
            return os;
        }
        std::vector<ItemCount *>::iterator begin_;
        std::vector<ItemCount *>::iterator end_;
        std::uint32_t bin_count_;
        std::uint32_t size_;

        std::uint32_t capacity(std::uint32_t bin_capacity) const {
            return bin_count_ * bin_capacity;
        }

     public:
        Block() = default;
        Block(std::vector<ItemCount *>::iterator begin,
              std::vector<ItemCount *>::iterator end, std::uint32_t bin_count,
              std::uint32_t size)
            : begin_{begin}, end_{end}, bin_count_{bin_count}, size_{size} {}
        void put(ItemCount *item, std::uint32_t bin_capacity) {
            size_ += item->size;
            if (size_ > capacity(bin_capacity)) {
                ++bin_count_;
            }
            ++end_;
        }
        std::uint32_t slack(std::uint32_t bin_capacity) const {
            return capacity(bin_capacity) - size_;
        }
        std::uint32_t score(std::uint32_t bin_capacity) const {
            return std::distance(begin_, end_) + slack(bin_capacity) +
                   bin_count_ - 1u;
        }
        std::uint32_t size() const { return size_; }
        std::pair<std::vector<ItemCount *>::iterator,
                  std::vector<ItemCount *>::iterator>
        items() const {
            return std::make_pair(begin_, end_);
        }
        std::uint32_t bin_count() const { return bin_count_; }
        static bool allowed(const Block &block, std::uint32_t bin_capacity,
                            std::uint32_t *slack) {
            const auto block_slack = block.slack(bin_capacity);

            if (block_slack > *slack) {
                return false;
            }

            const auto pair = block.items();

            auto it = std::find_if(pair.first, pair.second, [](auto *item) {
                if (item->count > 0u) {
                    --item->count;
                    return false;
                }
                return true;
            });

            if (it != pair.second) {
                for (auto rit = std::make_reverse_iterator(it);
                     rit != std::make_reverse_iterator(pair.first); ++rit) {
                    ++((*rit)->count);
                }
                return false;
            }

            *slack -= block_slack;

            return true;
        }
    };

    Solution() : items_{}, blocks_{}, age_{} {}
    Solution(const Solution &other)
        : items_(other.items()), blocks_{}, age_{other.age_} {
        blocks_.reserve(other.blocks().size());
        std::transform(other.blocks().cbegin(), other.blocks().cend(),
                       std::back_inserter(blocks_),
                       [&items = items_, &other ](const auto &b) {
                           auto pair = b.items();
                           return Block(items.begin() +=
                                        (pair.first - other.items().begin()),
                                        items.begin() +=
                                        (pair.second - other.items().begin()),
                                        b.bin_count(), b.size());
                       });
    }
    Solution(Solution &&other) = default;
    Solution &operator=(Solution &&other) = default;
    Solution &operator=(const Solution &other) {
        auto temp(other);
        return *this = std::move(temp);
    }
    std::uint32_t size() const { return blocks_.size(); }
    const std::vector<ItemCount *> &items() const { return items_; }
    std::vector<ItemCount *> &items() { return items_; }
    const std::vector<Block> &blocks() const { return blocks_; }
    std::vector<Block> &blocks() { return blocks_; }
    unsigned int age() const { return age_; }
    void increase_age(unsigned int increment = 1u) { age_ += increment; }

 private:
    std::vector<ItemCount *> items_;
    std::vector<Block> blocks_;
    unsigned int age_;

    template <intmax_t Num, intmax_t Dom, bool use_b3>
    friend void adaptive_mutation(Problem *problem, Solution *mutant);

    friend std::ostream &operator<<(std::ostream &os,
                                    const Solution &solution) {
        if (solution.size()) {
            auto it = solution.blocks().cbegin();
            os << *it;
            for (++it; it != solution.blocks().cend(); ++it) {
                os << ", " << *it;
            }
        }
        return os;
    }
};

} // namespace optimizer

#endif
