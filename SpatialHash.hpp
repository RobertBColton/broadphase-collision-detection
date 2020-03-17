/**
 * @file SparseSpatialBroadphase.hpp
 * @brief Implements a class for sparse spatial hashing collision detection.
 * @section License
 * Copyright (C) 2017 Robert Colton
 * License pending. All rights reserved.
 */

#ifndef SPARSESPATIALBROADPHASE_H
#define SPARSESPATIALBROADPHASE_H

#include "Broadphase.h"

#include <iterator>
#include <unordered_map>
#include <unordered_set>

struct Point {
	int x, y;
	Point(int x, int y) : x(x), y(y) {}

	friend inline bool operator==(Point const& lhs, Point const& rhs)
	{
		return (lhs.x == rhs.x) && (lhs.y == lhs.y);
	}
};

class SpatialHash : public Broadphase {
	typedef std::pair<void *const, void *const> CollisionPair;

	struct PointHash {
		inline std::size_t operator()(const Point &v) const {
			return v.x * 31 + v.y;
		}
	};

	struct CollisionPairHash {
		inline std::size_t operator()(const CollisionPair &v) const {
			uintptr_t ad = (uintptr_t) &v;
			return (size_t) ((13*ad) ^ (ad >> 15));
		}
	};

	int cell_width, cell_height;
	std::unordered_map<Point, std::unordered_set<Proxy*>, PointHash> cells;

public:
	SpatialHash() : cell_width(64), cell_height(64) {};
	SpatialHash(int cell_width, int cell_height) :
		cell_width(cell_width), cell_height(cell_height) {}

	void setCellSize(const int cell_width, const int cell_height) {
		this->cell_width = cell_width;
		this->cell_height = cell_height;
	}

	void setCellWidth(const int cell_width) {
		this->cell_width = cell_width;
	}

	void setCellHeight(const int cell_height) {
		this->cell_height = cell_height;
	}

	int getCellWidth() const {
		return cell_width;
	}

	int getCellHeight() const {
		return cell_height;
	}

	Proxy* addPoint(const int x, const int y, void *const userdata) {
		Proxy* proxy = new Proxy(AABB(x, y, 1, 1), userdata);
		cells[Point(x / cell_width, y / cell_height)].insert(proxy);
		return proxy;
	}

	Proxy* addRectangle(
			const int x, const int y, const int width, const int height, void *const userdata) {
		auto proxy = new Proxy(AABB(x, y, width, height), userdata);
		int xx = x / cell_width, yy = y / cell_height;
		for (int i = xx; i < ((x + width) / cell_width) + 1; ++i) {
			for (int ii = yy; ii < ((y + height) / cell_height) + 1; ++ii) {
				cells[Point(i, ii)].insert(proxy);
			}
		}
		return proxy;
	}

	Proxy* addProxy(AABB aabb, void* userdata = 0) {
		return addRectangle(aabb.getX(), aabb.getY(), aabb.getWidth(), aabb.getHeight(), userdata);
	}

	void removeProxy(Proxy* proxy) {
		int x = proxy->aabb.getX(), y = proxy->aabb.getY(),
				width = proxy->aabb.getWidth(), height = proxy->aabb.getHeight();
		int xx = x / cell_width, yy = y / cell_height;
		for (int i = xx; i < ((x + width) / cell_width) + 1; ++i) {
			for (int ii = yy; ii < ((y + height) / cell_height) + 1; ++ii) {
				cells[Point(i, ii)].erase(proxy);
			}
		}
		delete proxy;
	}

	std::unordered_set<Proxy*> queryRange(const int x, const int y, const int radius) {
		std::unordered_set<Proxy*> hits;
		const int xx = (x - radius) / cell_width, yy = (y - radius) / cell_height;
		const int diameter = radius * 2;
		for (int i = xx; i < ((x + diameter) / cell_width) + 1; ++i) {
			for (int ii = yy; ii < ((y + diameter) / cell_height) + 1; ++ii) {
				for (auto proxy : cells[Point(i, ii)]) {
					if (hits.count(proxy) > 0) continue;
					if (proxy->aabb.intersectsCircle(x, y, radius))
						hits.insert(proxy);
				}
			}
		}
		return hits;
	}

	const std::unordered_set<CollisionPair, CollisionPairHash> queryCollisionPairs() {
		std::unordered_set<CollisionPair, CollisionPairHash> collisionPairs;
		for (const auto &cell : cells) {
			for (auto proxyIt = cell.second.cbegin(); proxyIt != cell.second.cend();) {
				const auto &proxy = *proxyIt;
				for (auto otherIt = ++proxyIt; otherIt != cell.second.cend(); ++otherIt) {
					const auto &other = *otherIt;
					if (proxy->aabb.intersectsAABB(other->aabb))
						collisionPairs.insert(CollisionPair(proxy->userdata, other->userdata));
				}
			}
		}
		return collisionPairs;
	}

	void clear() {
		std::unordered_set<Proxy*> unique;
		for (auto cell : cells)
			for (auto proxy : cell.second)
				unique.insert(proxy);
		for (auto proxy : unique)
			delete proxy;
		std::unordered_map<Point, std::unordered_set<Proxy*>, PointHash>().swap(cells);
	}
};

#endif // SPARSESPATIALBROADPHASE_HPP
