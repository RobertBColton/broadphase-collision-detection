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
	virtual std::unordered_set<Proxy*> queryRange(const int x, const int y, const int radius) = 0;
};

#endif // BROADPHASE_H
