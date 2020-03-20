/**
 * @file SparseSpatialBroadphase.hpp
 * @brief Implements a class for sparse spatial hashing collision detection.
 * @section License
 * Copyright (C) 2017 Robert Colton
 * License pending. All rights reserved.
 */

#ifndef SPARSESPATIALBROADPHASE_H
#define SPARSESPATIALBROADPHASE_H

#include "Broadphase.hpp"

#include <iterator>
#include <unordered_map>
#include <vector>
#include <algorithm>

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
	std::unordered_map<Point, std::vector<Proxy*>, PointHash> cells;

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
		cells[Point(x / cell_width, y / cell_height)].push_back(proxy);
		return proxy;
	}

	Proxy* addRectangle(
			const int x, const int y, const int width, const int height, Proxy* proxy) {
		int xx = x / cell_width, yy = y / cell_height;
		for (int i = xx; i < ((x + width) / cell_width) + 1; ++i) {
			for (int ii = yy; ii < ((y + height) / cell_height) + 1; ++ii) {
				cells[Point(i, ii)].push_back(proxy);
			}
		}
		return proxy;
	}

	Proxy* addProxy(Proxy* proxy) {
		auto& aabb = proxy->aabb;
		return addRectangle(aabb.getX(), aabb.getY(), aabb.getWidth(), aabb.getHeight(), proxy);
	}

	void removeProxy(Proxy* proxy, bool free) {
		int x = proxy->aabb.getX(), y = proxy->aabb.getY(),
				width = proxy->aabb.getWidth(), height = proxy->aabb.getHeight();
		int xx = x / cell_width, yy = y / cell_height;
		for (int i = xx; i < ((x + width) / cell_width) + 1; ++i) {
			for (int ii = yy; ii < ((y + height) / cell_height) + 1; ++ii) {
				auto& cell = cells[Point(i, ii)];
				const auto it = std::find(cell.begin(), cell.end(), proxy);
				cell.erase(it);
			}
		}
		if (free) delete proxy;
	}

	std::vector<Proxy*> queryRange(const int x, const int y, const int radius) {
		std::vector<Proxy*> hits;
		const int xx = (x - radius) / cell_width, yy = (y - radius) / cell_height;
		for (int i = xx; i < ((x + radius) / cell_width) + 1; ++i) {
			for (int ii = yy; ii < ((y + radius) / cell_height) + 1; ++ii) {
				for (auto proxy : cells[Point(i, ii)]) {
					auto px = proxy->aabb.getX() / cell_width,
							 py = proxy->aabb.getY() / cell_height;
					// already looked at this proxy?
					if (std::max(px, xx) < i || std::max(py, yy) < ii) continue;
					if (proxy->aabb.intersectsCircle(x, y, radius))
						hits.push_back(proxy);
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
		for (auto& cell : cells)
			for (auto proxy : cell.second)
				unique.insert(proxy);
		for (auto proxy : unique)
			delete proxy;
		std::unordered_map<Point, std::vector<Proxy*>, PointHash>().swap(cells);
	}
};

#endif // SPARSESPATIALBROADPHASE_HPP
