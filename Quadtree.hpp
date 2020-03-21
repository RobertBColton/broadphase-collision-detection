/**
 * @file Quadtree.hpp
 * @brief Implements a class for hierarchical quadtree collision detection.
 * @section License
 * Copyright (C) 2020 Robert Colton
 * License pending. All rights reserved.
 */

#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include "Broadphase.hpp"

#include <unordered_set>

class Quadtree : public Broadphase
{
	struct Node {
		AABB aabb;
		Node *NW = nullptr, *NE = nullptr, *SE = nullptr, *SW = nullptr;
		std::unordered_set<Proxy*> children;

		Node(const AABB& aabb): aabb(aabb) {}
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
			std::unordered_set<Proxy*>().swap(children);
			if (NW) {
				NW->clear();
				NE->clear();
				SW->clear();
				SE->clear();
			}
		}

		void queryRange(const int x, const int y, const int radius, std::vector<Proxy*>& hits) {
			if (!aabb.intersectsCircle(x, y, radius)) return;
			for (auto child : children)
				if (child->aabb.intersectsCircle(x, y, radius))
					hits.push_back(child);
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

	void buildTree(Node* root, int cd = 0) {
		if (cd >= depth) return;
		int newWidth = root->aabb.getWidth() / 2,
				newHeight = root->aabb.getHeight() / 2;
		root->NW = new Node(AABB(root->aabb.getX(), root->aabb.getY(), newWidth, newHeight));
		root->NE = new Node(AABB(root->aabb.getX() + newWidth + 1, root->aabb.getY(), newWidth, newHeight));
		root->SW = new Node(AABB(root->aabb.getX(), root->aabb.getY() + newHeight + 1, newWidth, newHeight));
		root->SE = new Node(AABB(root->aabb.getX() + newWidth + 1, root->aabb.getY() + newHeight + 1, newWidth, newHeight));
		buildTree(root->NW, cd + 1);
		buildTree(root->NE, cd + 1);
		buildTree(root->SW, cd + 1);
		buildTree(root->SE, cd + 1);
	}

public:
	Quadtree(int width = 1024, int height = 1024, int depth = 4):
		Broadphase(), root(AABB(width, height)), depth(depth) {
		buildTree(&root);
	}

	Proxy* addProxy(Proxy* proxy) override {
		return root.addProxy(proxy);
	}

	void removeProxy(Proxy* proxy, bool free) override {
		root.removeProxy(proxy);
		if (free) delete proxy;
	}

	void clear() override { root.clear(); }

	std::vector<Proxy*> queryRange(const int x, const int y, const int radius) override {
		std::vector<Broadphase::Proxy*> hits;
		root.queryRange(x, y, radius, hits);
		return hits;
	}
};

#endif // QUADTREE_HPP
