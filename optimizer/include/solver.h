#ifndef SOLVER_H_
#define SOLVER_H_

#include <array>
#include <memory>

#include "operators.h"
#include "replacers.h"
#include "selectors.h"
#include "solution.h"

namespace optimizer {

/*
 * The `Solver` class solves a problem using the grouping genetic algorithm.
 */
template <std::uint32_t NP = 100u, std::uint32_t NC = 20u,
          std::uint32_t NM = 83u, std::uint32_t NE = 10u,
          std::uint32_t LS = 10u, std::uint32_t NG = 500u,
          std::uint32_t DL = 100u, typename k1 = std::ratio<13u, 10u>,
          typename k2 = std::ratio<4u, 1u>>
class Solver {
    Problem *problem_;

 public:
    explicit Solver(Problem *problem) : problem_(problem) {}
    Solver(const Solver &) = delete;
    Solver &operator=(const Solver &) = delete;
    Solution
    solve(std::array<std::unique_ptr<Solution>, NP> *population,
          std::uint32_t *gen = nullptr,
          std::vector<std::uint32_t> *blocks_over_time = nullptr) const {
        Solution best_solution(*(*population)[0]);
        auto generation = std::uint32_t{};
        auto previous = best_solution.size();
        auto delta_counter = std::uint32_t{};

        for (; generation < NG &&
               problem_->bin_count() - best_solution.size() >
                   problem_->lower_bound() &&
               delta_counter < DL;
             ++generation) {
            std::array<Solution *, NC / 2u> g;
            std::array<Solution *, NC / 2u> r;
            controlled_selection_crossover<NP, NC, NE>(problem_, population, &g,
                                                       &r);
            std::array<std::unique_ptr<Solution>, NC> progeny;

            for (auto i = 0u; i < NC / 2u; ++i) {
                progeny[i] = gene_level_crossover<true>(problem_, *g[i], *r[i]);
                progeny[i + NC / 2u] =
                    gene_level_crossover<true>(problem_, *r[i], *g[i]);
            }

            controlled_replacement_crossover<NP, NC, NE>(population, &progeny,
                                                         &r);

            std::vector<Solution *> clones;
            clones.reserve(NE);
            std::array<Solution *, NM> mutants;

            controlled_selection_mutation<NP, NM, NE, LS>(*population, &clones,
                                                          &mutants);

            std::sort(clones.begin(), clones.end());
            std::sort(mutants.begin(), mutants.end());

            std::vector<Solution *> pure;
            pure.reserve(mutants.size());

            std::set_difference(mutants.begin(), mutants.end(), clones.begin(),
                                clones.end(), std::back_inserter(pure));

            std::vector<std::unique_ptr<Solution>> cloned;
            cloned.reserve(clones.size());

            std::transform(clones.begin(), clones.end(),
                           std::back_inserter(cloned), [](const Solution *sol) {
                               return std::make_unique<Solution>(*sol);
                           });

            for (auto &sol : pure) {
                adaptive_mutation<k1::num, k1::den, true>(problem_, sol);
            }

            for (auto &sol : clones) {
                adaptive_mutation<k2::num, k2::den, true>(problem_, sol);
            }

            std::sort(population->begin(), population->end(),
                      [](const auto &left, const auto &right) {
                          return left->size() > right->size();
                      });

            if (!cloned.empty()) {
                controlled_replacement_mutation<NP>(population, &cloned);
            }

            const auto &current_best = (*population)[0];

            if (current_best->size() > best_solution.size()) {
                best_solution = *current_best;
            }

            if (previous == best_solution.size()) {
                ++delta_counter;
            } else {
                previous = best_solution.size();
                delta_counter = 0u;
            }

            for (auto i = 0u; i < NE; i++) {
                (*population)[i]->increase_age();
            }

            if (blocks_over_time) {
                blocks_over_time->push_back(best_solution.size());
            }
        }

        if (gen) {
            *gen = generation;
        }

        return best_solution;
    }
};

} // namespace optimizer

#endif

