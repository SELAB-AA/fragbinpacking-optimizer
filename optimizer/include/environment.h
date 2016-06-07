#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <random>

class Environment {
	unsigned int seed_;
	std::mt19937 rng_;
public:
	Environment();
	Environment(unsigned int seed);
	std::mt19937 *rng();
	unsigned int seed() const;
	void reseed();
	void reseed(unsigned int seed);
};

#endif
