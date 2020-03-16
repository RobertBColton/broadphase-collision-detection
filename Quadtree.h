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
		~Node() {
			for (auto proxy : children)
				delete proxy;
			delete NW;
			delete NE;
			delete SE;
			delete SW;
		}

		Proxy* addProxy(Proxy *proxy) {
			if (!aabb.intersectsAABB(proxy->aabb)) return nullptr;
			if (NW) {
				if (NW->aabb.containsAABB(proxy->aabb)) return NW->addProxy(proxy);
				if (NE->aabb.containsAABB(proxy->aabb)) return NE->addProxy(proxy);
				if (SW->aabb.containsAABB(proxy->aabb)) return SW->addProxy(proxy);
				if (SE->aabb.containsAABB(proxy->aabb)) return SE->addProxy(proxy);
			}
			children.insert(proxy);
			return proxy;
		}

		void removeProxy(Proxy* proxy) {
			if (!aabb.intersectsAABB(proxy->aabb)) return;
			if (children.erase(proxy) > 0) return;
			if (NW) {
				NW->removeProxy(proxy);
				NE->removeProxy(proxy);
				SW->removeProxy(proxy);
				SE->removeProxy(proxy);
			}
		}

		void clear() {
			for (auto proxy : children)
				delete proxy;
			std::set<Proxy*>().swap(children);
			if (NW) {
				NW->clear();
				NE->clear();
				SW->clear();
				SE->clear();
			}
		}

		void queryRange(const int x, const int y, const int radius, std::set<Proxy*>& hits) {
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
	Quadtree(int width = 1024, int height = 1024, int depth = 4);
	Proxy* addProxy(AABB aabb, void* userdata = 0) override;
	void removeProxy(Proxy* proxy) override;
	void clear() override;
	std::set<Proxy*> queryRange(const int x, const int y, const int radius) override;
};

#endif // QUADTREE_H
