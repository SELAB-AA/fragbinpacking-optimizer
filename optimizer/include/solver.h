#ifndef SOLVER_H_
#define SOLVER_H_

#include <array>
#include <iostream>
#include <memory>

#include "operators.h"
#include "problem.h"
#include "replacers.h"
#include "selectors.h"
#include "solution.h"

template<size_t NP = 100, size_t NC = 20, size_t NM = 83, size_t NE = 10, size_t LS = 10, size_t NG = 500, size_t DL = 10>
class Solver {
	Problem *problem_;
	const double k1_;
	const double k2_;
public:
	Solver(Problem *problem, double k1 = 1.3, double k2 = 4.0) : problem_(problem), k1_(k1), k2_(k2) {}
	Solver(const Solver&) = delete;
	Solver &operator=(const Solver&) = delete;
	Solution solve(std::array<std::shared_ptr<Solution>, NP> *population, size_t *gen = nullptr, std::vector<size_t> *blocks_over_time = nullptr) const {
		Solution best_solution(*(*population)[0].get());
		size_t generation = 0;
		auto previous = best_solution.size();
		size_t delta_counter = 0;

		// std::cout << "Generation " << generation << ": " << problem_->bin_count() - best_solution.size() << " / " << problem_->lower_bound() << "\n";

		for (; generation < NG && problem_->bin_count() - best_solution.size() > problem_->lower_bound() && delta_counter < DL; generation++) {
			std::array<std::shared_ptr<Solution>, NC / 2> g;
			std::array<std::shared_ptr<Solution>, NC / 2> r;
			controlled_selection_crossover<NP, NC, NE>(problem_, population, &g, &r);
			std::array<std::shared_ptr<Solution>, NC> progeny;

			/*
			for (const auto &sol : *population) {
				std::cout << sol->size() << " ";
			}

			std::cout << "\n";
			*/

			/*for (size_t i = 0; i < NC / 2; i++) {
				gene_level_crossover(problem_, g[i], r[i], &progeny[i << 1]);
				gene_level_crossover(problem_, r[i], g[i], &progeny[(i << 1) + 1]);
			}*/

			for (size_t i = 0; i < NC / 2; i++) {
				gene_level_crossover(problem_, g[i], r[i], &progeny[i]);
			}

			for (size_t i = 0; i < NC / 2; i++) {
				gene_level_crossover(problem_, r[i], g[i], &progeny[i + NC / 2]);
			}

			controlled_replacement_crossover<NP, NC, NE>(population, &progeny, &r);

			std::vector<std::shared_ptr<Solution>> clones;
			clones.reserve(NE);
			std::array<std::shared_ptr<Solution>, NM> mutants;

			controlled_selection_mutation<NP, NM, NE, LS>(*population, &clones, &mutants);

			std::sort(clones.begin(), clones.end());
			std::sort(mutants.begin(), mutants.end());

			std::vector<std::shared_ptr<Solution>> pure;
			pure.reserve(mutants.size());

			std::set_difference(mutants.begin(), mutants.end(), clones.begin(), clones.end(), std::back_inserter(pure));

			std::vector<std::shared_ptr<Solution>> cloned;
			cloned.reserve(clones.size());

			std::transform(clones.begin(), clones.end(), std::back_inserter(cloned), [](const auto &sol) { return std::make_shared<Solution>(*sol); });

			for (auto &sol : pure) {
				adaptive_mutation(problem_, sol, k1_);
			}

			for (auto &sol : clones) {
				adaptive_mutation(problem_, sol, k2_);
			}

			std::sort(population->begin(), population->end(), [](const auto &left, const auto &right) { return left->size() > right->size(); });

			if (!cloned.empty()) {
				controlled_replacement_mutation<NP>(population, &cloned);
			}

			const auto &current_best = (*population)[0];

			if (current_best->size() > best_solution.size()) {
				best_solution = *current_best;
			}

			if (previous == best_solution.size()) {
				delta_counter++;
			} else {
				previous = best_solution.size();
				delta_counter = 0;
			}

			for (size_t i = 0; i < NE; i++) {
				(*population)[i]->increase_age();
			}

			if (blocks_over_time) {
				blocks_over_time->push_back(best_solution.size());
			}
			// std::cout << "Generation " << generation + 1 << ": " << problem_->bin_count() - best_solution.size() << " / " << problem_->lower_bound() << "\n";
		}

		if (gen) {
			*gen = generation;
		}

		return best_solution;
	}
};

#endif
