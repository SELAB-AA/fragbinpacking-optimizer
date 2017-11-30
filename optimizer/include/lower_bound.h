#ifndef LOWER_BOUND_H_
#define LOWER_BOUND_H_

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <utility>
#include <vector>

namespace optimizer {

/*
 * Packs items into bins using First Fit with a maximum of two items per bin.
 * Uses a segment tree for efficient implementation in O(n log n).
 */
class Fitter {
 public:
    Fitter(std::uint32_t n, std::uint32_t c)
        : n_(n), c_(c), st_(n << 2u), bins_() {
        bins_.reserve(n);
    }

    constexpr static auto mid(std::uint32_t left, std::uint32_t right) {
        return (left + right) >> 1u;
    }

    std::uint32_t query(std::uint32_t idx, std::uint32_t val,
                        std::uint32_t left, std::uint32_t right) {
        if (left == right) {
            return left;
        }

        if (st_[2u * idx] <= c_ - val) {
            return query(2u * idx, val, left, mid(left, right));
        } else {
            return query(2u * idx + 1u, val, mid(left, right) + 1u, right);
        }
    }

    void update(std::uint32_t idx, std::uint32_t x, std::uint32_t val,
                std::uint32_t left, std::uint32_t right) {
        if (left == right) {
            st_[idx] += val;
            return;
        }

        const auto m = mid(left, right);

        if (x <= m) {
            update(2u * idx, x, val, left, m);
        } else {
            update(2u * idx + 1u, x, val, m + 1u, right);
        }

        st_[idx] = std::min(st_[2u * idx], st_[2u * idx + 1u]);
    }

    void fit(std::uint32_t val) {
        const auto idx = query(1u, val, 1u, n_);
        if (idx > bins_.size()) {
            bins_.push_back(std::make_pair(val, 0u));
            update(1u, idx, val, 1u, n_);
        } else {
            bins_[idx - 1u].second = val;
            update(1u, idx, c_ - bins_[idx - 1u].first, 1u, n_);
        }
    }

    std::vector<std::pair<std::uint32_t, std::uint32_t>> &bins() {
        return bins_;
    }

 private:
    std::uint32_t n_;
    std::uint32_t c_;
    std::vector<std::uint32_t> st_;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> bins_;
};

/*
 * Efficient unsigned division by 3.
 */
constexpr std::uint32_t div3u(std::uint32_t n) {
    return static_cast<std::uint32_t>(0xaaaaaaabull * n >> 33u);
}

/*
 * Step function used by bound class L_*^(p), which is part of `l3star`.
 */
constexpr std::uint32_t u(std::uint32_t k, std::uint32_t x, std::uint32_t c) {
    const auto quot = x * (k + 1u) / c;
    const auto rem = x * (k + 1u) % c;
    return rem == 0u ? x * k : quot * c;
}

/*
 * Computes the bound L_3^* for a problem defined by the `ItemCount` range
 * from begin to end, the amount slack, the bin count and their capacity.
 * The iterations parameter corresponds to the constant p in bound
 * class L_*^(p).
 */
template <std::uint32_t iterations = 20u, class InputIt>
inline static std::uint32_t l3star(InputIt begin, InputIt end,
                                   std::uint32_t slack, std::uint32_t bin_count,
                                   std::uint32_t bin_capacity) {
    if (bin_count <= 1u) {
        return 0u;
    }
    const auto n = std::accumulate(
        begin, end, std::uint32_t{},
        [](auto lhs, const auto &rhs) { return lhs += rhs.count; });

    if (n <= bin_count) {
        return 0u;
    }

    auto x = std::uint32_t{}, y = std::uint32_t{};
    auto infeasible = false;

    if (slack != 0u) {
        auto s = slack;
        Fitter f(n, bin_capacity);
        for (auto rbegin = std::make_reverse_iterator(end),
                  rend = std::make_reverse_iterator(begin);
             rbegin != rend; ++rbegin) {
            for (auto i = 0u; i < rbegin->count; ++i) {
                const auto d = bin_capacity - rbegin->size;
                if (s >= d) {
                    if (++x == bin_count) {
                        return 0u;
                    }
                    s -= d;
                }
                f.fit(rbegin->size);
            }
        }

        auto &bins = f.bins();

        const auto new_end = std::remove_if(
            bins.begin(), bins.end(), [](const auto &v) { return !v.second; });

        std::sort(bins.begin(), new_end, [](const auto &l, const auto &r) {
            return l.first + l.second > r.first + r.second;
        });

        const auto slack_used_by_x = slack - s;

        s = slack;

        for (auto current = bins.begin(); current != new_end; ++current) {
            const auto sum = current->first + current->second;
            const auto d = bin_capacity - sum;
            if (s < d) {
                break;
            }
            if (++y == bin_count) {
                return 0u;
            }
            s -= d;
        }

        const auto slack_used_by_y = slack - s;
        infeasible = slack_used_by_x + slack_used_by_y > slack;
    }

    const auto right = std::upper_bound(
        begin, end, bin_capacity / 2u,
        [](auto lhs, const auto &rhs) { return lhs < rhs.size; });

    const auto possible_blocks = div3u(n + 2u * x + y - infeasible);

    const auto minsplit =
        bin_count > possible_blocks ? bin_count - possible_blocks : 0u;

    if (begin == right && slack == 0u) {
        return std::max(minsplit, n - bin_count);
    }

    auto kmax = std::uint32_t{};

    for (auto k = 2u; k <= iterations; ++k) {
        auto total = std::accumulate(
            begin, end, std::uint32_t{},
            [k, bin_capacity](auto lhs, const auto &rhs) {
                return lhs + rhs.count * u(k, rhs.size, bin_capacity);
            });
        auto maximum = total ? 1u + ((total - 1u) / (bin_capacity * k)) : total;

        auto rit = std::make_reverse_iterator(end);
        auto it = begin;
        auto prev = begin;
        if (it != right) {
            while (rit->size > bin_capacity - it->size) {
                total += rit->count *
                         (bin_capacity * k - u(k, rit->size, bin_capacity));
                ++rit;
            }
            const auto ceiling =
                total ? 1u + (total - 1u) / (bin_capacity * k) : total;
            maximum = std::max(ceiling, maximum);
            ++it;
        }
        for (; it != right; ++it) {
            while (rit->size > bin_capacity - it->size) {
                total += rit->count *
                         (bin_capacity * k - u(k, rit->size, bin_capacity));
                ++rit;
            }

            total -= prev->count * u(k, prev->size, bin_capacity);
            ++prev;

            const auto ceiling =
                total ? 1u + (total - 1u) / (bin_capacity * k) : total;
            if (ceiling < maximum) {
                break;
            }
            maximum = ceiling;
        }
        kmax = std::max(maximum, kmax);
    }

    auto total = std::accumulate(
        begin, end, std::uint32_t{},
        [](auto lhs, const auto &rhs) { return lhs + rhs.count * rhs.size; });

    auto l2maximum = total ? 1u + (total - 1u) / bin_capacity : total;
    auto rit = std::make_reverse_iterator(end);
    auto prev = begin;
    for (auto it = begin; it != right; ++it) {
        while (it->size > bin_capacity - rit->size) {
            total += rit->count * (bin_capacity - rit->size);
            ++rit;
        }
        if (it != begin) {
            total -= prev->count * prev->size;
            ++prev;
        }
        const auto ceiling = total ? 1u + (total - 1u) / bin_capacity : total;
        if (ceiling < l2maximum) {
            break;
        }
        l2maximum = ceiling;
    }

    const auto maxval = std::max(kmax, l2maximum);
    return maxval < bin_count ? minsplit
                              : std::max(minsplit, maxval - bin_count);
}

} // namespace optimizer

#endif

