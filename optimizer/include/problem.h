#ifndef PROBLEM_H_
#define PROBLEM_H_

#include <assert.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <numeric>
#include <vector>

#include "environment.h"
#include "lower_bound.h"
#include "replacers.h"
#include "threesum.h"
#include "util.h"

namespace optimizer {

/*
 * A `Problem` object contains the specifications of a problem which is
 * guaranteed to be reduced by E1 and E2 upon creation. Also has remaining
 * optimiztion algorithms.
 */
class Problem {
    mutable Environment *env_;
    mutable std::vector<ItemCount> items_;
    std::uint32_t bin_count_;
    std::uint32_t bin_capacity_;
    std::uint32_t item_count_;
    std::uint32_t original_bin_count_;
    std::uint32_t original_item_count_;
    std::uint32_t original_slack_;
    std::uint32_t unique_size_count_;
    std::uint32_t slack_;
    std::uint32_t lower_bound_;
    std::uint32_t optimal1_;
    std::uint32_t optimal21_;
    std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>>
        optimal22_;
    std::vector<Partition> initial_3_partitions_;
    bool solved_;

    /*
     * Determines if a partition is allowed with respect to the currently
     * available items and slack. The argument p_one points to the `ItemCount`
     * entry for size 1. If the partition is allowed, its items are copied
     * to the out iterator and the number of items copied is returned.
     */
    template <class OutputIt>
    static std::uint32_t
    allowed_partition(const Partition &partition, std::uint32_t *slack,
                      const ItemCount *p_one, OutputIt out) {
        auto idx = 0u;
        const auto &p_items = partition.items();

        if (p_items[0]->count == 0u) {
            return 0u;
        }
        --p_items[0]->count;
        ++idx;
        auto size = p_items[0]->size;

        auto used_slack = false;

        if (p_items[1] == p_one) {
            if (p_items[1]->count > 0u) {
                --p_items[1]->count;
                ++idx;
                size += p_items[1]->size;
            } else if (*slack) {
                --*slack;
                used_slack = true;
            } else {
                ++p_items[0]->count;
                return 0u;
            }
        } else {
            if (p_items[1]->count > 0u) {
                --p_items[1]->count;
                ++idx;
                size += p_items[1]->size;
            } else {
                ++p_items[0]->count;
                return 0u;
            }
        }

        if (p_items[2] == p_one) {
            if (p_items[2]->count > 0u) {
                --p_items[2]->count;
                ++idx;
                size += p_items[2]->size;
            } else if (*slack) {
                --*slack;
            } else {
                if (used_slack) {
                    ++*slack;
                } else {
                    ++p_items[1]->count;
                }
                ++p_items[0]->count;
                return 0u;
            }
        } else {
            if (p_items[2]->count > 0u) {
                --p_items[2]->count;
                ++idx;
                size += p_items[2]->size;
            } else {
                if (used_slack) {
                    ++*slack;
                } else {
                    ++p_items[1]->count;
                }
                ++p_items[0]->count;
                return 0u;
            }
        }
        std::copy_n(p_items.cbegin(), idx, out);
        return idx;
    }

    /*
     * The core of algorithm B3. Produces blocks from a set of partitions in the
     * range from begin to end. The slack argument is an in/out parameter for
     * the amount of slack available. The item_count argument is an in/out
     * parameter for the number of unpacked items. Argument p_one is a pointer
     * to the `ItemCount` object for item size 1, while the solution argument
     * points to the solution which should be processed. It returns the number
     * of bins used.
     */
    template <class RandomIt>
    std::uint32_t find_packing(RandomIt begin, RandomIt end,
                               std::uint32_t *slack, std::uint32_t *item_count,
                               ItemCount *p_one, Solution *solution) const {
        std::uint32_t s = std::distance(begin, end);
        auto bins_used = std::uint32_t{};

        while (s > 0u) {
            const auto idx = bounded_rand(s, *env_->rng());
            const auto n =
                allowed_partition(begin[idx], slack, p_one,
                                  std::back_inserter(solution->items()));
            if (n) {
                *item_count -= n;
                const auto size = std::accumulate(
                    begin[idx].items().cbegin(),
                    begin[idx].items().cbegin() + n, std::uint32_t{},
                    [](auto lhs, const auto &rhs) { return lhs += rhs->size; });
                const auto bin_count = size > bin_capacity_ ? 2u : 1u;
                bins_used += bin_count;
                solution->blocks().emplace_back(solution->items().end() -= n,
                                                solution->items().end(),
                                                bin_count, size);
            } else {
                std::swap(begin[idx], begin[--s]);
            }
        }

        return bins_used;
    }

    /*
     * The core of algorithm G^+. Finds blocks for a `Problem` given a randomly
     * permuted range of items from begin to end and the amount of slack
     * available. Found blocks are written to the out iterator.
     */
    template <class InIt, class OutIt>
    static void next_fit_fragmentation(const Problem &problem, InIt begin,
                                       InIt end, std::uint32_t slack,
                                       OutIt out) {
        if (begin == end) {
            return;
        }

        auto current_block = Solution::Block(begin, begin, 1u, 0u);
        auto has_slack = false;

        for (; begin != end; ++begin) {
            if (*begin) {
                const auto s = current_block.slack(problem.bin_capacity());
                if ((*begin)->size > s && has_slack * slack >= s) {
                    has_slack = false;
                    slack -= s;
                    current_block.complete();
                    *out++ = std::move(current_block);
                    current_block = Solution::Block(begin, begin, 1u, 0u);
                }
            } else {
                has_slack = true;
            }
            current_block.put(*begin, problem.bin_capacity());
        }
        slack -= current_block.slack(problem.bin_capacity());
        current_block.complete();
        *out++ = std::move(current_block);
        if (slack) {
            assert(slack % problem.bin_capacity() == 0u);
            std::fill_n(out, slack / problem.bin_capacity(),
                        Solution::Block(end, end, 1u, 0u));
        }
    }

    Problem(const Problem &) = delete;
    Problem &operator=(const Problem &) = delete;
    template <bool use_b3>
    friend std::unique_ptr<Solution> gene_level_crossover(Problem *problem,
                                                          const Solution &l,
                                                          const Solution &r);
    template <intmax_t Num, intmax_t Dom, bool use_b3>
    friend void adaptive_mutation(Problem *problem, Solution *mutant);

 public:
    template <class InputIt>
    Problem(Environment *env, InputIt begin, InputIt end,
            std::uint32_t bin_capacity, std::uint32_t bin_count = 0u)
        : env_{env}, bin_capacity_{bin_capacity},
          item_count_{static_cast<std::uint32_t>(std::distance(begin, end))},
          original_item_count_{item_count_}, optimal1_{}, optimal21_{},
          optimal22_{}, initial_3_partitions_{}, solved_{} {
        const auto sum = std::accumulate(begin, end, std::uint32_t{});

        assert(bin_capacity && "bad capacity");
        bin_count_ = 1u + (sum - 1u) / bin_capacity;

        if (bin_count) {
            assert(bin_count >= bin_count_ && "bad bin count");
            bin_count_ = bin_count;
        }

        if (bin_count_ >= item_count_ || bin_count_ < 2u) {
            solved_ = true;
        }

        original_bin_count_ = bin_count_;
        original_slack_ = slack_ = bin_count_ * bin_capacity - sum;

        std::vector<std::uint32_t> item_sizes_copy;
        item_sizes_copy.reserve(original_item_count_);

        for (; begin != end; ++begin) {
            if (*begin == bin_capacity) {
                ++optimal1_;
                continue;
            }
            if (*begin == bin_capacity - 1u && slack_ > 0u) {
                ++optimal21_;
                --slack_;
                continue;
            }
            item_sizes_copy.push_back(*begin);
        }

        bin_count_ -= optimal1_ + optimal21_;
        item_count_ -= optimal1_ + optimal21_;

        std::sort(item_sizes_copy.begin(), item_sizes_copy.end(),
                  std::isgreater<std::uint32_t, std::uint32_t>);

        items_ = fcount(item_sizes_copy.cbegin(), item_sizes_copy.cend());

        if (items_.size()) {
            auto l = items_.begin();
            auto r = --items_.end();

            optimal22_.reserve(item_sizes_copy.size() / 2u);

            while (l < r) {
                auto together = l->size + r->size;
                if (together == bin_capacity_) {
                    auto min = std::min(l->count, r->count);
                    l->count -= min;
                    r->count -= min;
                    optimal22_.emplace_back(min, l++->size, r--->size);
                    bin_count_ -= min;
                    item_count_ -= 2u * min;
                } else if (together < bin_capacity_) {
                    --r;
                } else {
                    ++l;
                }
            }

            if (l == r) {
                auto together = l->size + r->size;
                if (together == bin_capacity_) {
                    optimal22_.emplace_back(l->count / 2u, l->size, l->size);
                    bin_count_ -= l->count / 2u;
                    item_count_ -= l->count - l->count % 2u;
                    l->count = l->count % 2u;
                }
            }
        }

        if (bin_count_ >= item_count_ || bin_count_ < 2u) {
            solved_ = true;
        }

        optimal22_.shrink_to_fit();

        auto it = std::find_if(items_.begin(), items_.end(),
                               [](const auto &v) { return v.count == 0u; });
        if (it != items_.end()) {
            items_.resize(std::distance(
                items_.begin(),
                std::copy_if(it + 1, items_.end(), it,
                             [](const auto &v) { return v.count > 0u; })));
        }

        lower_bound_ = l3star(items_.crbegin(), items_.crend(), slack_,
                              bin_count_, bin_capacity_);

        unique_size_count_ = items_.size();

        if (items_.size() && items_.back().size != 1u && slack_) {
            // add entry since there is slack
            items_.emplace_back(1u, 0u);
        }

        items_.shrink_to_fit();

        threesum(items_.begin(), items_.end(), &initial_3_partitions_, 1u,
                 bin_capacity);
        threesum(items_.begin(), items_.end(), &initial_3_partitions_, 2u,
                 bin_capacity);
    }
    Environment *env() const { return env_; }
    std::vector<ItemCount> &items() { return items_; }
    const std::vector<ItemCount> &items() const { return items_; }
    std::uint32_t bin_count() const { return bin_count_; }
    std::uint32_t bin_capacity() const { return bin_capacity_; }
    std::uint32_t item_count() const { return item_count_; }
    std::uint32_t unique_size_count() const { return unique_size_count_; }
    std::uint32_t original_bin_count() const { return original_bin_count_; }
    std::uint32_t original_item_count() const { return original_item_count_; }
    std::uint32_t original_slack() const { return original_slack_; }
    std::uint32_t slack() const { return slack_; }
    std::uint32_t lower_bound() const { return lower_bound_; }
    bool solved() const { return solved_; }
    /*
     * Produces blocks from the `ItemCount` objects in the range from begin to
     * end. The slack argument is an in/out parameter for the amount of slack
     * available. The item_count argument is an in/out parameter for the number
     * of unpacked items. The solution argument points to the solution which
     * should be processed. Returns the number of bins used.
     */
    template <class InputIt>
    std::uint32_t b3(InputIt begin, InputIt end, std::uint32_t *slack,
            std::uint32_t *item_count, Solution *solution) const {
        if (begin == end) {
            return 0u;
        }
        std::vector<Partition> partitions;
        partitions.reserve(initial_3_partitions_.size());
        threesum(begin, end, &partitions, 1u, bin_capacity_);
        threesum(begin, end, &partitions, 2u, bin_capacity_);

        return find_packing(partitions.begin(), partitions.end(), slack,
                            item_count, &*--end, solution);
    }
    /*
     * Shuffles the range of items from begin to end and finds blocks therein,
     * given the amount of slack available. Found blocks are written to the out
     * iterator.
     */
    template <class RandomIt, class OutputIt>
    constexpr void g(RandomIt begin, RandomIt end, std::uint32_t *slack,
                     OutputIt out) const {
        if (begin == end) {
            return;
        }
        optimizer::shuffle(begin, end, *env_->rng());
        const auto slack_in = *slack;
        *slack = 0u;
        next_fit_fragmentation(*this, begin, end, slack_in, out);
    }
    /*
     * Produces an initial solution. If parameter do_b3 is `true`, algorithm
     * B_3 G^+ is used, else only G^+ is used.
     */
    template <bool do_b3 = true>
    std::unique_ptr<Solution> generate_individual() {
        auto result = std::make_unique<Solution>();
        const auto items_copy(items_);
        auto item_count(item_count_);
        const auto max_blocks = bin_count_ - lower_bound_;
        auto bin_count(bin_count_);

        auto slack(slack_);
        result->items().reserve(item_count + bin_count - 1u);
        result->blocks().reserve(max_blocks);

        if (do_b3) {
            bin_count -= find_packing(
                initial_3_partitions_.begin(), initial_3_partitions_.end(),
                &slack, &item_count, &items_.back(), result.get());
        }

        if (item_count != 0u) {
            for (auto &v : items_) {
                std::fill_n(std::back_inserter(result->items()), v.count, &v);
            }
            const auto dummies = bin_count - 1u;
            std::fill_n(std::back_inserter(result->items()), dummies, nullptr);

            g(result->items().end() -= item_count + dummies,
              result->items().end(), &slack,
              std::back_inserter(result->blocks()));
        }

        std::sort(result->blocks().begin(), result->blocks().end(),
                  [c = bin_capacity_](const auto &l, const auto &r) {
                      return l.score(c) < r.score(c);
                  });

        std::copy(items_copy.cbegin(), items_copy.cend(), items_.begin());
        return result;
    }
};

} // namespace optimizer

#endif

