/**
 * @file SparseSpatialBroadphase.hpp
 * @brief Implements a class for sparse spatial hashing collision detection.
 * @section License
 * Copyright (C) 2017 Robert Colton
 * License pending. All rights reserved.
 */

#ifndef SPARSESPATIALBROADPHASE_H
#define SPARSESPATIALBROADPHASE_H

#include "AxisAlignedBoundingBox.hpp"

#include <iterator>
#include <unordered_map>
#include <unordered_set>

struct Point {
	int x, y;
	Point(int x, int y) : x(x), y(y) {};

	friend inline bool operator==(Point const& lhs, Point const& rhs)
	{
		return (lhs.x == rhs.x) && (lhs.y == lhs.y);
	}
};

class SparseSpatialBroadphase {
	typedef std::pair<void *const, const AABB> Proxy;
	typedef std::pair<void *const, void *const> CollisionPair;

	struct PointHash {
		inline std::size_t operator()(const Point &v) const {
			return v.x * 31 + v.y;
		}
	};

	struct ProxyHash {
		inline std::size_t operator()(const Proxy &v) const {
			uintptr_t ad = (uintptr_t) &v;
			return (size_t) ((13*ad) ^ (ad >> 15));
		}
	};

	struct CollisionPairHash {
		inline std::size_t operator()(const CollisionPair &v) const {
			uintptr_t ad = (uintptr_t) &v;
			return (size_t) ((13*ad) ^ (ad >> 15));
		}
	};

	int cell_width, cell_height;
	std::unordered_map<Point, std::unordered_set<Proxy, ProxyHash>, PointHash> cells;

public:
	SparseSpatialBroadphase() : cell_width(1), cell_height(1) {};
	SparseSpatialBroadphase(int cell_width, int cell_height) :
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

	void addPoint(const int x, const int y, void *const userdata) {
		cells[Point(x / cell_width, y / cell_height)].insert(Proxy(userdata, AABB(x, y, 1, 1)));
	}

	void addRectangle(
			const int x, const int y, const int width, const int height, void *const userdata) {
		int xx = x / cell_width, yy = y / cell_height;
		for (int i = xx; i < ((x + width) / cell_width) + 1; ++i) {
			for (int ii = yy; ii < ((y + height) / cell_height) + 1; ++ii) {
				cells[Point(i, ii)].insert(Proxy(userdata, AABB(x, y, width, height)));
			}
		}
	}

	const std::unordered_set<CollisionPair, CollisionPairHash> getCollisionPairs() {
		std::unordered_set<CollisionPair, CollisionPairHash> collisionPairs;
		for (const auto &cell : cells) {
			for (auto proxyIt = cell.second.cbegin(); proxyIt != cell.second.cend();) {
				const auto &proxy = *proxyIt;
				for (auto otherIt = ++proxyIt; otherIt != cell.second.cend(); ++otherIt) {
					const auto &other = *otherIt;
					if (proxy.second.intersectsAABB(other.second)) {
						collisionPairs.insert(CollisionPair(proxy.first, other.first));
					}
				}
			}
		}
		return collisionPairs;
	}

	void clear() {
		cells.clear();
	}
};

#endif // SPARSESPATIALBROADPHASE_HPP
