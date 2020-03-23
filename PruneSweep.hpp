/**
 * @file PruneSweep.hpp
 * @brief Implements a class for sweep and prune collision detection.
 * @section License
 * Copyright (C) 2020 Robert Colton
 * License pending. All rights reserved.
 */

#ifndef PRUNESWEEP_H
#define PRUNESWEEP_H

#include "Broadphase.hpp"

/*
 * Prune Sweep Sucks
 *
 * I spent some time fiddling with this algorithm before I realized its absolutely pointless nature
 * akin to that of the kd tree or any of the other dynamic tree solutions to the problem of broadphase.
 * Starting out with the use of a vector, your update complexity can not beat O(n log n) as it
 * essentially has to resort the proxies. The time could be improved using a linked list allowing
 * constant insertion and removal to move the proxies in the list during sort, but then binary search
 * becomes impossible and degenerates to O(n). A skip list is thus the next logical step but is the
 * same as a binary space partition at that point, which I also ruled out as useless due to its
 * similarity to the quadtree!
 *
 * Summary: Prune Sweep -> A poor man's binary space partition. Consider this class simply the base time
 * for comparison in the benchmark table.
 *
 */

class PruneSweep : public Broadphase {

public:
	PruneSweep() {};

	Proxy* addPoint(const int x, const int y, void *const userdata) {
		return 0;
	}

	Proxy* addRectangle(
			const int x, const int y, const int width, const int height, Proxy* proxy) {
		return 0;
	}

	Proxy* addProxy(Proxy* proxy) override {
		return 0;
	}

	void removeProxy(Proxy* proxy, bool free) override {

	}

	// TODO: throw out
	virtual void updateProxy(Proxy* proxy, const AABB& aabb) override {}

	std::vector<Proxy*> queryRange(const int x, const int y, const int radius) override {
		std::vector<Proxy*> hits;

		return hits;
	}

	void clear() override {

	}
};

#endif // PRUNESWEEP_H
