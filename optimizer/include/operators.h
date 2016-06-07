#ifndef OPERATORS_H_
#define OPERATORS_H_

#include <memory>

class Problem;
class Solution;

void gene_level_crossover(Problem *problem, const std::shared_ptr<Solution> &l, const std::shared_ptr<Solution> &r, std::shared_ptr<Solution> *p);
void adaptive_mutation(Problem *problem, const std::shared_ptr<Solution> &mutant, double k);

#endif
