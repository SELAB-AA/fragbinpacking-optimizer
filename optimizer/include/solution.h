#ifndef SOLUTION_H_
#define SOLUTION_H_

#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

class Block;
class Problem;

class Solution {
	std::shared_ptr<const Problem> problem_;
	std::vector<std::shared_ptr<Block>> blocks_;
	unsigned int age_;

	friend void adaptive_mutation(Problem *problem, const std::shared_ptr<Solution> &mutant, double k);

	friend void swap(Solution &lhs, Solution &rhs) {
		lhs.swap(rhs);
	}

	void swap(Solution &solution) {
		using std::swap;

		swap(problem_, solution.problem_);
		swap(blocks_, solution.blocks_);
		swap(age_, solution.age_);
	}

	friend std::ostream& operator<<(std::ostream& os, const Solution& solution);
public:
	Solution(const Problem &problem);
	Solution(const Problem &problem, std::vector<std::shared_ptr<Block>> &&blocks);
	Solution(const Solution &solution);
	Solution(Solution &&solution) = default;
	Solution &operator=(const Solution &solution);
	Solution &operator=(Solution &&solution) = default;
	~Solution() = default;
	size_t size() const;
	const std::vector<std::shared_ptr<Block>> &blocks() const;
	const unsigned int age() const;
	void increase_age(unsigned int age = 1);
};

#endif
