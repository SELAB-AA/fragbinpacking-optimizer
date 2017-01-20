#ifndef OPERATORS_H_
#define OPERATORS_H_

#include <assert.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <ratio>

#include "environment.h"
#include "solution.h"
#include "util.h"

namespace optimizer {

/*
 * Performs grouping crossover on two parent `Solution`s, l and r, belonging to
 * a `Problem`. Parameter `use_b3` indicates if algorithm B3 should be employed.
 *
 * Returns the `Solution` resulting from combining l with r.
 */
template <bool use_b3>
std::unique_ptr<Solution> inline gene_level_crossover(Problem *problem,
                                                      const Solution &l,
                                                      const Solution &r) {
    auto result = std::make_unique<Solution>();
    const auto items_copy(problem->items());
    auto item_count(problem->item_count());
    auto slack(problem->slack());

    const auto &ll = l.blocks();
    const auto &rr = r.blocks();

    result->items().reserve(item_count);
    result->blocks().reserve((item_count + slack) / 3u);

    auto aa = ll.cbegin();
    auto bb = rr.cbegin();
    auto cc = std::back_inserter(result->items());
    auto dd = std::back_inserter(result->blocks());

    if (ll.size() > rr.size()) {
        const auto d = ll.size() - rr.size();
        for (const auto end = aa + d; aa != end; ++aa) {
            assert(
                Solution::Block::allowed(*aa, problem->bin_capacity(), &slack));
            const auto pair = aa->items();
            const auto delta = std::distance(pair.first, pair.second);
            item_count -= delta;
            std::copy(pair.first, pair.second, cc);
            dd = Solution::Block(result->items().end() -= delta,
                                 result->items().end(), aa->bin_count(),
                                 aa->size());
        }
    } else if (rr.size() > ll.size()) {
        const auto d = rr.size() - ll.size();
        for (const auto end = bb + d; bb != end; ++bb) {
            assert(
                Solution::Block::allowed(*bb, problem->bin_capacity(), &slack));
            const auto pair = bb->items();
            const auto delta = std::distance(pair.first, pair.second);
            item_count -= delta;
            std::copy(pair.first, pair.second, cc);
            dd = Solution::Block(result->items().end() -= delta,
                                 result->items().end(), bb->bin_count(),
                                 bb->size());
        }
    }

    while (aa != ll.end()) {
        if (aa->score(problem->bin_capacity()) <=
            bb->score(problem->bin_capacity())) {
            if (Solution::Block::allowed(*aa, problem->bin_capacity(),
                                         &slack)) {
                const auto pair = aa->items();
                const auto delta = std::distance(pair.first, pair.second);
                item_count -= delta;
                std::copy(pair.first, pair.second, cc);
                dd = Solution::Block(result->items().end() -= delta,
                                     result->items().end(), aa->bin_count(),
                                     aa->size());
            }
            ++aa;
            if (Solution::Block::allowed(*bb, problem->bin_capacity(),
                                         &slack)) {
                const auto pair = bb->items();
                const auto delta = std::distance(pair.first, pair.second);
                item_count -= delta;
                std::copy(pair.first, pair.second, cc);
                dd = Solution::Block(result->items().end() -= delta,
                                     result->items().end(), bb->bin_count(),
                                     bb->size());
            }
            ++bb;
        } else {
            if (Solution::Block::allowed(*bb, problem->bin_capacity(),
                                         &slack)) {
                const auto pair = bb->items();
                const auto delta = std::distance(pair.first, pair.second);
                item_count -= delta;
                std::copy(pair.first, pair.second, cc);
                dd = Solution::Block(result->items().end() -= delta,
                                     result->items().end(), bb->bin_count(),
                                     bb->size());
            }
            ++bb;
            if (Solution::Block::allowed(*aa, problem->bin_capacity(),
                                         &slack)) {
                const auto pair = aa->items();
                const auto delta = std::distance(pair.first, pair.second);
                item_count -= delta;
                std::copy(pair.first, pair.second, cc);
                dd = Solution::Block(result->items().end() -= delta,
                                     result->items().end(), aa->bin_count(),
                                     aa->size());
            }
            ++aa;
        }
    }

    if (item_count != 0u) {
        if (use_b3) {
            if (item_count >= problem->item_count() - 6) {
                problem->find_packing(problem->initial_3_partitions_.begin(),
                                      problem->initial_3_partitions_.end(),
                                      &slack, &item_count,
                                      &problem->items_.back(), result.get());
            } else {
                problem->b3(problem->items_.begin(), problem->items_.end(),
                            &slack, &item_count, result.get());
            }
        }
        if (item_count != 0u) {
            for (auto &v : problem->items_) {
                std::fill_n(std::back_inserter(result->items()), v.count, &v);
            }

            problem->g(result->items().end() -= item_count,
                       result->items().end(), &slack,
                       std::back_inserter(result->blocks()));
        }
    }

    std::sort(
        result->blocks().begin(), result->blocks().end(),
        [c = problem->bin_capacity()](const auto &left, const auto &right) {
            return left.score(c) < right.score(c);
        });

    std::copy(items_copy.cbegin(), items_copy.cend(), problem->items_.begin());
    return result;
}

/*
 * Mutates a `Solution` belonging to a `Problem` in place.
 * Parameters `Num` and `Den` form the numerator and denominator of k, the
 * constant for the aggressiveness of the mutation. Parameter `use_b3` indicates
 * whether B3 should be used or not.
 */
template <intmax_t Num, intmax_t Den, bool use_b3>
inline void adaptive_mutation(Problem *problem, Solution *mutant) {
    const auto &m = mutant->size();
    const auto max_blocks = problem->bin_count() - problem->lower_bound();

    if (max_blocks == m) {
        return;
    }

    assert(max_blocks > m);

    const auto f = 0.1;
    const auto p = std::pow(0.5 - static_cast<double>(m) / (2.0 * max_blocks),
                            1.0 / (static_cast<double>(Num) / Den));
    const auto a = (1.0 - f) / f * p;
    const auto b = (1.0 - f) / f * (1.0 - p);
    static std::uniform_real_distribution<> dist{};
    const auto u = 1.0 - dist(*problem->env()->rng());
    const auto q = std::pow(1.0 - u, 1.0 / b);
    const auto p_e = std::pow(1.0 - q, 1.0 / a);
    const auto n_b = std::max(static_cast<std::uint32_t>(std::ceil(m * p_e)),
                              std::uint32_t{1u});
    assert(n_b <= m);

    const auto items_copy(problem->items());

    for (auto &item : problem->items_) {
        item.count = 0u;
    }

    auto slack = 0u;

    auto item_count = 0u;

    std::for_each(
        sample_inplace(++mutant->blocks_.rbegin(), mutant->blocks_.rend(),
                       n_b - 1u, *problem->env()->rng())
            .base(),
        mutant->blocks_.end(),
        [&slack, &item_count, c = problem->bin_capacity() ](const auto &block) {
            auto pair = block.items();
            for (; pair.first != pair.second; ++pair.first) {
                ++(*pair.first)->count;
                ++item_count;
            }
            slack += block.slack(c);
        });

    std::vector<ItemCount *> new_items;
    new_items.reserve(mutant->items_.size());
    std::vector<Solution::Block> new_blocks;
    new_blocks.reserve(mutant->blocks_.capacity());

    for (auto it = mutant->blocks_.cbegin(); it != mutant->blocks_.cend() - n_b;
         ++it) {
        const auto range = it->items();
        std::copy(range.first, range.second, std::back_inserter(new_items));
    }

    mutant->items_ = std::move(new_items);

    auto count = 0u;
    for (auto it = mutant->blocks_.cbegin(); it != mutant->blocks_.cend() - n_b;
         ++it) {
        const auto range = it->items();
        const auto d = std::distance(range.first, range.second);
        new_blocks.emplace_back(mutant->items_.begin() += count,
                                mutant->items_.begin() += count + d,
                                it->bin_count(), it->size());
        count += d;
    }
    mutant->blocks_ = std::move(new_blocks);

    if (use_b3) {
        if (item_count >= problem->item_count() - 6) {
            problem->find_packing(problem->initial_3_partitions_.begin(),
                                  problem->initial_3_partitions_.end(), &slack,
                                  &item_count, &problem->items_.back(), mutant);
        } else {
            problem->b3(problem->items_.begin(), problem->items_.end(), &slack,
                        &item_count, mutant);
        }
    }

    if (item_count) {
        for (auto &v : problem->items_) {
            std::fill_n(std::back_inserter(mutant->items_), v.count, &v);
        }

        problem->g(mutant->items_.end() -= item_count, mutant->items_.end(),
                   &slack, std::back_inserter(mutant->blocks_));
    }

    mutant->age_ = 0u;

    std::copy(items_copy.cbegin(), items_copy.cend(), problem->items_.begin());
}

} // namespace optimizer

#endif
