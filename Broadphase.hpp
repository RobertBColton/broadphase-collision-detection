#ifndef BROADPHASE_H
#define BROADPHASE_H

#include "AxisAlignedBoundingBox.hpp"

#include <unordered_set>

class Broadphase {
protected:
	Broadphase() {}
public:

	struct Proxy {
		void* userdata;
		AABB aabb;
		Proxy(void* userdata = nullptr): userdata(userdata) {}
		Proxy(AABB aabb, void* userdata = nullptr): userdata(userdata), aabb(aabb) {}
	};

	virtual ~Broadphase() {}
	virtual Proxy* addProxy(AABB aabb, void* userdata = 0) = 0;
	virtual void removeProxy(Proxy* proxy) = 0;
	virtual void clear() = 0;
	virtual std::unordered_set<Proxy*> queryRange(const int x, const int y, const int radius) = 0;
};

#endif // BROADPHASE_H
