#include <assert.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <vector>
#include <sstream>
#include <string>

#include "environment.h"
#include "problem.h"
#include "solution.h"
#include "solver.h"

const size_t POPULATION_SIZE = 100;

auto read_problems(std::istream &is) {
	std::vector<std::vector<size_t>> problems;

	for (std::string s; std::getline(is >> std::ws, s);) {
		std::stringstream ss(s);

		if (ss.peek() != '#') {
			size_t size;
			problems.emplace_back();
			while (ss >> size) {
				problems.back().push_back(size);
			}
		}
	}

	return problems;
}


int main() {
	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
	Environment env;

	const int runs = 10;
	const int NP = 100;

	std::array<int, 3> service_counts{256, 512, 1024};

	std::array<int, 3> capacities{8, 16, 32};

	for (const auto &c : capacities) {
		for (auto x = -2; x < 3; ++x) {
			const auto p = x * c;
			auto low = p / 3 + ((p % 3) >> ((sizeof (p) << 3) - 1));
			auto high = low + c;
			low = std::max(1, low + 1);
			high = std::min(c, high);
			for (const auto &n : service_counts) {
				std::stringstream ss;
				ss << "problems/uniform_" << c << "_" << low << "_" << high << "_" << n;
				std::ifstream is(ss.str());
				auto problems = read_problems(is);
				for (auto r = 0u; r < problems.size(); ++r) {
					auto e1e2_start = std::chrono::high_resolution_clock::now();
					Problem problem(&env, problems[r], c);
					std::chrono::duration<double> duration_e1e2 = std::chrono::high_resolution_clock::now() - e1e2_start;
					for (auto i = 0; i < runs; ++i) {
						// reset seed
						env.reseed();
						unsigned int seed = env.seed();

						auto g_start = std::chrono::high_resolution_clock::now();
						// run g
						auto solution_g = problem.generate_individual(false);
						std::chrono::duration<double> duration_g = std::chrono::high_resolution_clock::now() - g_start;

						// reset seed
						env.reseed(seed);

						auto b3g_start = std::chrono::high_resolution_clock::now();
						// run b3g
						auto solution_b3g = problem.generate_individual();
						std::chrono::duration<double> duration_b3g = std::chrono::high_resolution_clock::now() - b3g_start;

						// reset seed
						env.reseed(seed);

						// do genetic
						std::array<std::shared_ptr<Solution>, NP> population;
						Solution solution_stage1(problem);
						bool found_optimal = false;

						auto stage1_start = std::chrono::high_resolution_clock::now();
						for (size_t j = 0; j < population.size(); ++j) {
							population[j] = problem.generate_individual();

							if (problem.bin_count() - population[j]->size() == problem.lower_bound()) {
								solution_stage1 = std::move(*population[j]);
								found_optimal = true;
								break;
							}
						}

						std::chrono::duration<double> duration_stage1;

						if (!found_optimal) {
							std::sort(population.begin(), population.end(), [](const auto &left, const auto &right) { return left->size() > right->size(); });
							solution_stage1 = *population[0];
							duration_stage1 = std::chrono::high_resolution_clock::now() - stage1_start;
						} else {
							duration_stage1 = std::chrono::high_resolution_clock::now() - stage1_start;
						}

						auto solution_stage2 = solution_stage1;

						std::chrono::duration<double> duration_stage2{};

						size_t gen = 0;

						std::vector<size_t> blocks_over_time;
						blocks_over_time.push_back(solution_stage1.size());

						if (!found_optimal) {
							auto stage2_start = std::chrono::high_resolution_clock::now();
							solution_stage2 = Solver<NP>(&problem).solve(&population, &gen, &blocks_over_time);
							duration_stage2 = std::chrono::high_resolution_clock::now() - stage2_start;
						}

						std::stringstream dat_name;
						dat_name  << "results/uniform_" << c << "_" << low << "_" << high << "_" << n << "_" << r << "_" << i << ".dat";
						std::stringstream gen_name;
						gen_name  << "results/uniform_" << c << "_" << low << "_" << high << "_" << n << "_" << r << "_" << i << ".gen";

						std::ofstream of;
						of.open(dat_name.str(), std::ios::out | std::ios::trunc);

						assert(of.is_open());

						of << "# Seed: " << seed << "\n" <<
							"# Item count before reduction: " << n << "\n" <<
							"# Item count after reduction: " << problem.item_count() << "\n" <<
							"# Time spent in reduction: " << duration_e1e2.count() << "\n" <<
							"# Bin count: " << problem.bin_count() << "\n" <<
							"# Lower bound: " << problem.lower_bound() << "\n" <<
							"# Upper bound: " << problem.bin_count() - 1 << "\n" <<
							"# " << "\n" <<
							"# Format:" << "\n" <<
							"# blocks splits duration" << "\n" <<
							"# " << "\n" <<
							"# Order:" << "\n" <<
							"# G" << "\n" <<
							"# B3G" << "\n" <<
							"# FFF Stage 1" << "\n" <<
							"# FFF Stage 2" << "\n" <<
							solution_g->size() << " " << problem.bin_count() - solution_g->size() << " " << duration_g.count() << "\n" <<
							solution_b3g->size() << " " << problem.bin_count() - solution_b3g->size() << " " << duration_b3g.count() << "\n" <<
							solution_stage1.size() << " " << problem.bin_count() - solution_stage1.size() << " " << duration_stage1.count() << "\n" <<
							solution_stage2.size() << " " << problem.bin_count() - solution_stage2.size() << " " << duration_stage2.count() << "\n";

						of.close();

						of.open(gen_name.str(), std::ios::out | std::ios::trunc);

						assert(of.is_open());

						of << "# Blocks for generations of FFF, including generation 0\n";

						for (const auto &count : blocks_over_time) {
							of << count << "\n";
						}

						of.close();
					}
				}
			}
		}
	}
	return 0;
}
