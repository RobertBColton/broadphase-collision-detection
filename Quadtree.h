#ifndef QUADTREE_H
#define QUADTREE_H

#include "Broadphase.h"
#include "AxisAlignedBoundingBox.hpp"

#include <set>

class Quadtree : public Broadphase
{
	struct Node {
		AABB aabb;
		Node *NW = nullptr, *NE = nullptr, *SE = nullptr, *SW = nullptr;
		std::set<Proxy*> children;

		Node(AABB aabb): aabb(aabb) {}

		Proxy* addProxy(AABB aabb) {
			if (!aabb.intersectsAABB(this->aabb)) return nullptr;
			if (NW) {
				if (NW->aabb.containsAABB(aabb)) return NW->addProxy(aabb);
				if (NE->aabb.containsAABB(aabb)) return NE->addProxy(aabb);
				if (SW->aabb.containsAABB(aabb)) return SW->addProxy(aabb);
				if (SE->aabb.containsAABB(aabb)) return SE->addProxy(aabb);
			}
			Proxy* proxy = new Proxy(aabb);
			children.insert(proxy);
			return proxy;
		}

		void removeProxy(Proxy* proxy) {
			if (!proxy->aabb.intersectsAABB(this->aabb)) return;
			if (children.erase(proxy) > 0) return;
			if (NW) {
				NW->removeProxy(proxy);
				NE->removeProxy(proxy);
				SW->removeProxy(proxy);
				SE->removeProxy(proxy);
			}
		}

		void queryRange(const int x, const int y, const int radius, std::set<Proxy*> hits) {
			if (!aabb.intersectsCircle(x, y, radius)) return;
			for (auto child : children)
				if (child->aabb.intersectsCircle(x, y, radius))
					hits.insert(child);
			if (NW) {
				NW->queryRange(x, y, radius, hits);
				NE->queryRange(x, y, radius, hits);
				SW->queryRange(x, y, radius, hits);
				SE->queryRange(x, y, radius, hits);
			}
		}
	};

	Node root;
	int depth;

	void buildTree(Node* root, int cd = 0);

public:
	Quadtree(int width = 1000, int height = 1000, int depth = 4);
	Proxy* addProxy(AABB aabb) override;
	void removeProxy(Proxy* proxy) override;
	std::set<Proxy*> queryRange(const int x, const int y, const int radius) override;
};

#endif // QUADTREE_H
