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

	Proxy* addProxy(Proxy* proxy) {
		return 0;
	}

	void removeProxy(Proxy* proxy, bool free) {

	}

	// TODO: throw out
	virtual void updateProxy(Proxy* proxy, const AABB& aabb) {}

	std::vector<Proxy*> queryRange(const int x, const int y, const int radius) {
		std::vector<Proxy*> hits;

		return hits;
	}

	void clear() {

	}
};

#endif // PRUNESWEEP_H
