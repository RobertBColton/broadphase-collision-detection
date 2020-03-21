/**
 * @file SpatialHash.hpp
 * @brief Implements a class for sparse spatial hashing collision detection.
 * @section License
 * Copyright (C) 2017, 2020 Robert Colton
 * License pending. All rights reserved.
 */

#ifndef SPARSESPATIALBROADPHASE_HPP
#define SPARSESPATIALBROADPHASE_HPP

#include "Broadphase.hpp"

#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

struct Point {
	int x, y;
	Point(int x, int y) : x(x), y(y) {}

	friend inline bool operator==(Point const& lhs, Point const& rhs)
	{
		return (lhs.x == rhs.x) && (lhs.y == rhs.y);
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
	using CellBucket = std::pair<std::vector<Proxy*>,std::vector<Proxy*>>;
	std::unordered_map<Point, CellBucket, PointHash> cells;

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
		cells[Point(x / cell_width, y / cell_height)].first.push_back(proxy);
		return proxy;
	}

	Proxy* addRectangle(
			const int x, const int y, const int width, const int height, Proxy* proxy) {
		int xx = x / cell_width, yy = y / cell_height;
		for (int i = xx; i < ((x + width) / cell_width) + 1; ++i) {
			for (int ii = yy; ii < ((y + height) / cell_height) + 1; ++ii) {
				auto& cell = cells[Point(i, ii)];
				const bool origin = (i == xx && ii == yy);
				auto& cellProxies = origin ? cell.first : cell.second;
				cellProxies.push_back(proxy);
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
				const bool origin = (i == xx && ii == yy);
				auto& cellProxies = origin ? cell.first : cell.second;
				const auto it = std::find(cellProxies.begin(), cellProxies.end(), proxy);
				cellProxies.erase(it);
			}
		}
		if (free) delete proxy;
	}

	std::vector<Proxy*> queryRange(const int x, const int y, const int radius) {
		std::vector<Proxy*> hits;
		const int xx = (x - radius) / cell_width, yy = (y - radius) / cell_height;
		for (int i = xx; i < ((x + radius) / cell_width) + 1; ++i) {
			for (int ii = yy; ii < ((y + radius) / cell_height) + 1; ++ii) {
				auto& cell = cells[Point(i, ii)];
				for (auto proxy : cell.first) {
					if (proxy->aabb.intersectsCircle(x, y, radius))
						hits.push_back(proxy);
				}
				for (auto proxy : cell.second) {
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
			const auto& origin = cell.second.first;
			const auto& foreign = cell.second.second;

			for (auto proxyIt = origin.cbegin(); proxyIt != origin.cend();) {
				const auto &proxy = *proxyIt;
				// compare to all other origin proxies
				for (auto otherIt = ++proxyIt; otherIt != origin.cend(); ++otherIt) {
					const auto &other = *otherIt;
					if (proxy->aabb.intersectsAABB(other->aabb))
						collisionPairs.insert(CollisionPair(proxy->userdata, other->userdata));
				}
				// compare to all foreign proxies
				for (auto otherIt = foreign.cbegin(); otherIt != foreign.cend(); ++otherIt) {
					const auto &other = *otherIt;
					if (proxy->aabb.intersectsAABB(other->aabb))
						collisionPairs.insert(CollisionPair(proxy->userdata, other->userdata));
				}
			}

			// compare all foreign proxies to each other
			for (auto proxyIt = foreign.cbegin(); proxyIt != foreign.cend();) {
				const auto &proxy = *proxyIt;
				for (auto otherIt = ++proxyIt; otherIt != foreign.cend(); ++otherIt) {
					const auto &other = *otherIt;
					if (proxy->aabb.intersectsAABB(other->aabb))
						collisionPairs.insert(CollisionPair(proxy->userdata, other->userdata));
				}
			}
		}
		return collisionPairs;
	}

	void clear() {
		for (auto& cell : cells)
			for (auto proxy : cell.second.first)
				delete proxy;
		std::unordered_map<Point, CellBucket, PointHash>().swap(cells);
	}
};

#endif // SPARSESPATIALBROADPHASE_HPP
