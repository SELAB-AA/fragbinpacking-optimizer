#include "threesum.h"

#include <numeric>

void threesum(const std::vector<size_t> &s, size_t r, std::vector<Partition<size_t>> *out) {
	if (!r) {
		return;
	}

	auto n = s.size();
	auto ub_a = r - 2 * s[n - 1];
	auto lb_a = 1 + (r - 1) / 3;

	for (size_t i = s[0] > ub_a ? 1 : 0; i < n - 1; ++i) {
		auto a = s[i];
		if (a < lb_a) {
			break;
		}

		auto start = i;
		auto end = n - 1;
		auto trg = r - a;

		while (start <= end) {
			auto b = s[start];
			auto c = s[end];
			auto t = b + c;
			if (t == trg) {
				if (a == b) {
					if (b == c) {
						out->emplace_back(Partition<size_t>{size_t{1}, ItemCount<size_t>{c, 3}});
					} else {
						out->emplace_back(Partition<size_t>{size_t{2}, {ItemCount<size_t>{b, 2}, ItemCount<size_t>{c, 1}}});
					}
				} else {
					if (b == c) {
						out->emplace_back(Partition<size_t>{size_t{2}, {ItemCount<size_t>{a, 1}, ItemCount<size_t>{b, 2}}});
					} else {
						out->emplace_back(Partition<size_t>{size_t{3}, {ItemCount<size_t>{a, 1}, ItemCount<size_t>{b, 1}, ItemCount<size_t>{c, 1}}});
					}
				}
				++start;
			} else if (t > trg) {
				++start;
			} else {
				--end;
			}
		}
	}
}
