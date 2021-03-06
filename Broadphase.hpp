/**
 * @file Broadphase.hpp
 * @brief Defines an abstract base interface to be used for broadphase implementations.
 * @section License
 * Copyright (C) 2020 Robert Colton
 * License pending. All rights reserved.
 */

#ifndef BROADPHASE_HPP
#define BROADPHASE_HPP

#include "AABB.hpp"

#include <vector>

class Broadphase {
protected:
	Broadphase() {}
public:

	struct Proxy {
		void* userdata;
		AABB aabb;
		Proxy(void* userdata = nullptr): userdata(userdata) {}
		Proxy(const AABB& aabb, void* userdata = nullptr): userdata(userdata), aabb(aabb) {}
	};

	virtual ~Broadphase() {}
	virtual Proxy* addProxy(Proxy* proxy) = 0;
	virtual Proxy* addProxy(const AABB& aabb, void* userdata = 0) {
		Proxy* proxy = new Proxy(aabb, userdata);
		Proxy* res = addProxy(proxy);
		if (!res) delete proxy;
		return res;
	}
	virtual void removeProxy(Proxy* proxy, bool free = true) = 0;
	virtual void updateProxy(Proxy* proxy, const AABB& aabb) {
		removeProxy(proxy, false);
		proxy->aabb = aabb;
		addProxy(proxy);
	}
	virtual void clear() = 0;
	virtual std::vector<Proxy*> queryRange(const int x, const int y, const int radius) = 0;
};

#endif // BROADPHASE_HPP
