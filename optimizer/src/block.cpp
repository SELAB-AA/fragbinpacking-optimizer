#include "block.h"

#include <algorithm>

#include "problem.h"


Block::Block(const Problem &problem) : problem_(problem), bin_count_(1), size_(0), item_count_(0) {}

size_t Block::capacity() const {
	return bin_count_ * problem_.bin_capacity();
}

void Block::put(size_t item) {
	size_ += item;
	if (size_ > capacity()) {
		++bin_count_;
	}
	++items_[item];
	++item_count_;
}

size_t Block::slack() const {
	return capacity() - size_;
}

size_t Block::score() const {
	return item_count_ + slack() + bin_count_ - 1;
}

const std::unordered_map<size_t, size_t>& Block::items() const {
	return items_;
}

const size_t Block::bin_count() const {
	return bin_count_;
}

const size_t Block::item_count() const {
	return item_count_;
}

void Block::swap(std::unordered_map<std::size_t, std::size_t> *f, size_t a, size_t b, size_t c, std::vector<std::shared_ptr<Block>> *out, size_t *slack, bool slack_swap) const {
	*slack = slack_swap ? *slack - 1 : *slack + c - (a + b);

	if (bin_count_ < 2 || item_count() + this->slack() < (slack_swap ? 6 : 7)) {
		out->push_back(std::make_shared<Block>(*this));
		auto &new_block = out->back();
		new_block->size_ = slack_swap ? size_ - 1 : size_ + (c - (a + b));
		--new_block->item_count_;
		auto &items = new_block->items_;

		if (--items[a] == 0) {
			items.erase(a);
		}

		if (--items[b] == 0) {
			items.erase(b);
		}

		if (--(*f)[c] == 0) {
			f->erase(c);
		}

		++(*f)[a];
		++(*f)[b];
		++items[c];
	} else {
		auto block_slack = slack_swap ? this->slack() + 1 : this->slack() - (c - (a + b));
		auto items(items_);

		if (--items[a] == 0) {
			items.erase(a);
		}

		if (--items[b] == 0) {
			items.erase(b);
		}

		if (--(*f)[c] == 0) {
			f->erase(c);
		}

		++(*f)[a];
		++(*f)[b];
		++items[c];

		const bool b3 = false;
		if (b3) {
			problem_.b3(&items, &block_slack, out);
		}
		problem_.g(&items, &block_slack, out);
	}
}

void Block::swap(std::unordered_map<std::size_t, std::size_t> *f, size_t a, size_t b, size_t c, size_t d, std::vector<std::shared_ptr<Block>> *out, size_t *slack) const {
	*slack += (c + d) - (a + b);
	if (bin_count_ < 2 || item_count() + this->slack() < 6) {
		out->push_back(std::make_shared<Block>(*this));
		auto &new_block = out->back();
		new_block->size_ += ((c + d) - (a + b));
		auto &items = new_block->items_;

		if (--items[a] == 0) {
			items.erase(a);
		}

		if (--items[b] == 0) {
			items.erase(b);
		}

		if (--(*f)[c] == 0) {
			f->erase(c);
		}

		if (--(*f)[d] == 0) {
			f->erase(d);
		}

		++(*f)[a];
		++(*f)[b];
		++items[c];
		++items[d];
	} else {
		auto block_slack = this->slack() - ((c + d) - (a + b));
		auto items(items_);

		if (--items[a] == 0) {
			items.erase(a);
		}

		if (--items[b] == 0) {
			items.erase(b);
		}

		if (--(*f)[c] == 0) {
			f->erase(c);
		}

		if (--(*f)[d] == 0) {
			f->erase(d);
		}

		++(*f)[a];
		++(*f)[b];
		++items[c];
		++items[d];

		const bool b3 = false;
		if (b3) {
			problem_.b3(&items, &block_slack, out);
		}
		problem_.g(&items, &block_slack, out);
	}
}

std::ostream& operator<<(std::ostream& os, const Block& block) {
	std::vector<const std::pair<const size_t, size_t> *> items;
	items.reserve(block.items().size());
	for (const auto &item: block.items()) {
		items.push_back(&item);
	}
	std::sort(items.begin(), items.end(), [](const auto &l, const auto &r) { return l->first > r->first; });
	auto it = items.cbegin();
	os << "(" << (*it)->first << "^" << (*it)->second;
	for (++it; it != items.cend(); ++it) {
		os << ", " << (*it)->first << "^" << (*it)->second;
	}
	os << ")";
	return os;
}
