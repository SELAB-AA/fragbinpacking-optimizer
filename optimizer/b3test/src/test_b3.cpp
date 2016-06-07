#include <assert.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <vector>

#include "environment.h"
#include "problem.h"
#include "solution.h"
#include "solver.h"

int main() {
	Environment env;
	auto item_counts = std::array<size_t, 6>{1000, 2000, 4000, 8000, 16000, 32000};

	std::ofstream of;
	of.open("b3_time.csv", std::ios::out | std::ios::trunc);
	assert(of.is_open());

	of << "M\tM'\tN\tN'\tW\tT\n";

	for (const auto &item_count : item_counts) {
		const auto &bin_capacity = item_count;
		std::uniform_int_distribution<size_t> size_dist(1, bin_capacity);
		for (size_t i = 0; i < 5; ++i) {
			std::vector<size_t> item_sizes(item_count);
			std::generate(item_sizes.begin(), item_sizes.end(), [&size_dist, &env] {return size_dist(*env.rng());});
			Problem problem(&env, item_sizes, bin_capacity);
			auto items(problem.items());
			size_t slack = problem.slack();
			std::vector<std::shared_ptr<Block>> blocks;
			std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
			problem.b3(&items, &slack, &blocks);
			std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;
			of << problem.original_bin_count() << "\t" <<
				problem.bin_count() << "\t" <<
				problem.original_item_count() << "\t" <<
				problem.item_count() << "\t" <<
				problem.items().size() << "\t" <<
				duration.count() << "\n";
		}
	}
	of.close();
}
