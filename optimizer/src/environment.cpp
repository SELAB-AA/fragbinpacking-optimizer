#include "environment.h"

Environment::Environment() : seed_(std::random_device{}()), rng_(seed_) {}
Environment::Environment(unsigned int seed) : seed_(seed), rng_(seed_) {}

std::mt19937 *Environment::rng() {
	return &rng_;
}

unsigned int Environment::seed() const {
	return seed_;
}

void Environment::reseed() {
	reseed(std::random_device{}());
}

void Environment::reseed(unsigned int seed) {
	seed_ = seed;
	rng_.seed(seed);
}
