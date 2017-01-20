#ifndef PROBLEM_H_
#define PROBLEM_H_

#include <assert.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <numeric>
#include <vector>

#include "environment.h"
#include "replacers.h"
#include "threesum.h"
#include "util.h"

namespace optimizer {

/*
 * A `Problem` object contains the specifications of a problem which is
 * guaranteed to be reduced by E1 and E2 upon creation. Also has remaining
 * optimiztion algorithms and the lower bound.
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

    /*
     * Step function used by bound class L_*^(p), which is part of `l3star`.
     */
    static constexpr std::uint32_t u(std::uint32_t k, std::uint32_t x,
                                     std::uint32_t c) {
        const auto quot = x * (k + 1u) / c;
        const auto rem = x * (k + 1u) % c;
        return rem == 0u ? x * k : quot * c;
    }

    /*
     * Computes the bound L_3^* for a problem defined by the `ItemCount` range
     * from begin to end, the amount slack, the bin count and their capacity.
     * The iterations parameter corresponds to the constant p in bound
     * class L_*^(p).
     */
    template <std::uint32_t iterations = 20u, class InputIt>
    constexpr static std::uint32_t
    l3star(InputIt begin, InputIt end, std::uint32_t slack,
           std::uint32_t bin_count, std::uint32_t bin_capacity) {
        const auto right = std::upper_bound(
            begin, end, bin_capacity / 2u,
            [](auto lhs, const auto &rhs) { return lhs < rhs.size; });
        auto n =
            std::accumulate(begin, end, slack, [](auto lhs, const auto &rhs) {
                return lhs += rhs.count;
            });

        if (n - slack <= bin_count) {
            return 0u;
        }

        auto possible_blocks = n / 3;
        auto minsplit =
            bin_count > possible_blocks ? bin_count - possible_blocks : 0u;

        if (begin == right && slack == 0u) {
            auto diff = n - bin_count;
            return minsplit > diff ? minsplit : diff;
        }

        auto kmax = std::uint32_t{};

        for (auto k = 2u; k < iterations + 1u; ++k) {
            auto total = std::accumulate(
                begin, end, std::uint32_t{},
                [k, bin_capacity](auto lhs, const auto &rhs) {
                    return lhs + rhs.count * u(k, rhs.size, bin_capacity);
                });
            auto maximum =
                total ? 1u + ((total - 1u) / (bin_capacity * k)) : 0u;

            auto rit = std::make_reverse_iterator(end);
            auto prev = begin;
            for (auto it = begin; it != right; ++it) {
                while (it->size > bin_capacity - rit->size) {
                    total += rit->count *
                             (bin_capacity * k - u(k, rit->size, bin_capacity));
                    ++rit;
                }
                if (it != begin) {
                    total -= prev->count * u(k, prev->size, bin_capacity);
                    ++prev;
                }
                auto ceiling =
                    total ? 1u + ((total - 1u) / (bin_capacity * k)) : 0u;
                if (ceiling < maximum) {
                    break;
                }
                maximum = ceiling;
            }
            kmax = maximum > kmax ? maximum : kmax;
        }

        auto total = std::accumulate(begin, end, std::uint32_t{},
                                     [](auto lhs, const auto &rhs) {
                                         return lhs + rhs.count * rhs.size;
                                     });

        auto l2maximum = total ? 1u + ((total - 1u) / bin_capacity) : 0u;
        auto rit = std::make_reverse_iterator(end);
        auto prev = begin;
        for (auto it = begin; it != right; ++it) {
            while (it->size > bin_capacity - rit->size) {
                total += rit->count * (bin_capacity - rit->size);
                ++rit;
            }
            if (it != begin) {
                total -= prev->count * prev->size;
                ++prev;
            }
            auto ceiling = total ? 1u + ((total - 1u) / bin_capacity) : 0u;
            if (ceiling < l2maximum) {
                break;
            }
            l2maximum = ceiling;
        }

        auto maxval = std::max(kmax, l2maximum);
        return maxval < bin_count ? minsplit
                                  : std::max(minsplit, maxval - bin_count);
    }

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
     * of blocks made.
     */
    template <class RandomIt>
    std::uint32_t find_packing(RandomIt begin, RandomIt end,
                               std::uint32_t *slack, std::uint32_t *item_count,
                               ItemCount *p_one, Solution *solution) const {
        std::uint32_t s = std::distance(begin, end);

        auto blocks_made = 0u;

        while (s > 0u) {
            const auto idx = (*(env_->rng()))(s);
            const auto n =
                allowed_partition(begin[idx], slack, p_one,
                                  std::back_inserter(solution->items()));
            if (n) {
                *item_count -= n;
                auto size = std::accumulate(
                    begin[idx].items().cbegin(),
                    begin[idx].items().cbegin() + n, std::uint32_t{},
                    [](auto lhs, const auto &rhs) { return lhs += rhs->size; });
                auto bin_count = 1 + ((size - 1) / bin_capacity_);
                solution->blocks().emplace_back(solution->items().end() - n,
                                                solution->items().end(),
                                                bin_count, size);
                ++blocks_made;
            } else {
                std::swap(begin[idx], begin[--s]);
            }
        }

        return blocks_made;
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

        auto n = std::distance(begin, end);
        auto slack_reserve = 0u;

        static std::gamma_distribution<> gamma_dist{};
        static std::binomial_distribution<std::uint32_t> bino_dist{};

        auto item_counter = 1u;

        for (; begin != end; ++begin) {
            if ((*begin)->size > current_block.slack(problem.bin_capacity())) {
                if (current_block.slack(problem.bin_capacity()) >
                        slack_reserve &&
                    slack) {
                    auto x = gamma_dist(*problem.env()->rng(),
                                        std::gamma_distribution<>::param_type(
                                            item_counter, 1.0));
                    auto beta =
                        x /
                        (x + gamma_dist(*problem.env()->rng(),
                                        std::gamma_distribution<>::param_type(
                                            n + item_counter - 1, 1.0)));
                    auto slack_change = bino_dist(
                        *problem.env()->rng(),
                        std::binomial_distribution<std::uint32_t>::param_type(
                            slack, beta));
                    assert(slack_change <= slack);
                    slack_reserve += slack_change;
                    slack -= slack_change;
                    item_counter = 0;
                }

                if (current_block.slack(problem.bin_capacity()) <=
                    slack_reserve) {
                    slack_reserve -=
                        current_block.slack(problem.bin_capacity());
                    *out++ = std::move(current_block);
                    current_block = Solution::Block(begin, begin, 1u, 0u);
                }
            }
            current_block.put(*begin, problem.bin_capacity());
            ++item_counter;
            --n;
        }
        *out++ = std::move(current_block);
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
          optimal22_{}, initial_3_partitions_{} {
        auto sum = std::accumulate(begin, end, std::uint32_t{});

        auto rounded = sum - 1u - (sum - 1u) % bin_capacity + bin_capacity;
        bin_count_ = rounded / bin_capacity;

        if (bin_count) {
            assert(bin_count >= bin_count_ && "bad bin count");
            bin_count_ = bin_count;
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

        if (items_.back().size != 1u && slack_) {
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
    /*
     * Produces blocks from the `ItemCount` objects in the range from begin to
     * end. The slack argument is an in/out parameter for the amount of slack
     * available. The item_count argument is an in/out parameter for the number
     * of unpacked items. The solution argument points to the solution which
     * should be processed.
     */
    template <class InputIt>
    void b3(InputIt begin, InputIt end, std::uint32_t *slack,
            std::uint32_t *item_count, Solution *solution) const {
        if (begin == end) {
            return;
        }
        std::vector<Partition> partitions;
        partitions.reserve(initial_3_partitions_.size());
        threesum(begin, end, &partitions, 1u, bin_capacity_);
        threesum(begin, end, &partitions, 2u, bin_capacity_);

        find_packing(partitions.begin(), partitions.end(), slack, item_count,
                     &*--end, solution);
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
        pcg_extras::shuffle(begin, end, *env_->rng());
        auto slack_in = *slack;
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

        auto slack(slack_);
        result->items().reserve(item_count);
        result->blocks().reserve((item_count_ + slack) / 3u);

        if (do_b3) {
            find_packing(initial_3_partitions_.begin(),
                         initial_3_partitions_.end(), &slack, &item_count,
                         &items_.back(), result.get());
        }

        if (item_count != 0u) {
            for (auto &v : items_) {
                std::fill_n(std::back_inserter(result->items()), v.count, &v);
            }
            g(result->items().end() -= item_count, result->items().end(),
              &slack, std::back_inserter(result->blocks()));
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
