#include "problem.h"

#include <iostream>

#include "environment.h"

static size_t test_lb(Environment *env, const std::vector<std::pair<size_t, size_t> > &data, size_t c, size_t m) {
	std::vector<size_t> v;
	size_t count = 0;
	for (auto e : data) {
		v.resize(count + e.second);
		std::fill_n(v.begin() + count, e.second, e.first);
		count += e.second;
	}
	return Problem(env, v, c, m).lower_bound();
}

int main() {
	Environment env;

	std::cout << test_lb(&env, {std::make_pair(1, 10), std::make_pair(2, 4), std::make_pair(3, 22), std::make_pair(4, 1)}, 8, 11) << "\n";
	std::cout << test_lb(&env, {std::make_pair(2, 4), std::make_pair(4, 1), std::make_pair(7, 4)}, 8, 5) << "\n";
	std::cout << test_lb(&env, {std::make_pair(2, 4), std::make_pair(4, 1)}, 8, 2) << "\n";
	std::cout << test_lb(&env, {std::make_pair(2, 4), std::make_pair(4, 1)}, 8, 3) << "\n";
	std::cout << test_lb(&env, {std::make_pair(1, 12), std::make_pair(2, 2), std::make_pair(3, 40)}, 8, 17) << "\n";
	std::cout << test_lb(&env, {std::make_pair(2, 5)}, 5, 2) << "\n";
	std::cout << test_lb(&env, {std::make_pair(1, 28), std::make_pair(3, 12), std::make_pair(6, 8)}, 8, 14) << "\n";
	std::cout << test_lb(&env, {std::make_pair(5, 4), std::make_pair(6, 5), std::make_pair(7, 2)}, 8, 8) << "\n";
	std::cout << test_lb(&env, {std::make_pair(3, 1), std::make_pair(7, 1), std::make_pair(11, 1), std::make_pair(33, 3), std::make_pair(50, 1), std::make_pair(60, 1), std::make_pair(70, 1)}, 100, 3) << "\n";
	std::cout << test_lb(&env, {std::make_pair(3, 5), std::make_pair(6, 1), std::make_pair(7, 5)}, 8, 7) << "\n";
	std::cout << test_lb(&env, {std::make_pair(7, 8)}, 8, 7) << "\n";
	std::cout << test_lb(&env, {std::make_pair(3, 2), std::make_pair(7, 6)}, 8, 6) << "\n";
	std::cout << test_lb(&env, {std::make_pair(2, 1), std::make_pair(4, 3), std::make_pair(6, 6), std::make_pair(8, 5)}, 9, 10) << "\n";
	std::cout << test_lb(&env, {std::make_pair(2, 18)}, 9, 4) << "\n";
	std::cout << test_lb(&env, {std::make_pair(1, 16), std::make_pair(2, 24), std::make_pair(3, 64)}, 16, 16) << "\n";
	std::cout << test_lb(&env, {std::make_pair(2, 8), std::make_pair(3, 64)}, 16, 13) << "\n";
	std::cout << test_lb(&env, {std::make_pair(2, 8), std::make_pair(3, 16)}, 16, 4) << "\n";
	std::cout << test_lb(&env, {std::make_pair(5, 16)}, 16, 5) << "\n";
	std::cout << test_lb(&env, {std::make_pair(2, 1000), std::make_pair(33, 6000)}, 100, 2000) << "\n";
	std::cout << test_lb(&env, {std::make_pair(3, 200), std::make_pair(5, 200)}, 16, 100) << "\n";

	return 0;
}

