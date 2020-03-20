/**
 * @file AxisAlignedBoundingBox.hpp
 * @brief Implements a class for axis-aligned bounding box representation.
 * @section License
 * Copyright (C) 2017, 2020 Robert Colton
 * License pending. All rights reserved.
 */

#ifndef AXISALIGNEDBOUNDINGBOX_HPP
#define AXISALIGNEDBOUNDINGBOX_HPP

#include <algorithm>

class AABB {
	int x, y, width, height;

public:
	AABB(): x(0), y(0), width(0), height(0) {}
	AABB(int width, int height) : x(0), y(0), width(width), height(height) {}
	AABB(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}

	int getX() const { return x; }
	int getY() const { return y; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }

	void setPosition(const int x, const int y) {
		this->x = x;
		this->y = y;
	}

	void setX(const int x) { this->x = x; }
	void setY(const int y) { this->y = y; }

	void setSize(const int width, const int height) {
		this->width = width;
		this->height = height;
	}

	void setWidth(const int width) { this->width = width; }
	void setHeight(const int height) { this->height = height; }

	bool intersectsPoint(const int x, const int y) const {
		return x >= this->x && x <= this->x + this->width &&
				y >= this->y && y <= this->y + this->height;
	}

	bool point_in_circle(const int x, const int y, const int cx, const int cy, const int radius) {
		int xd = cx - x;
		int yd = cy - y;
		return (xd*xd + yd*yd) <= radius*radius;
	}

	bool intersectsCircle(const int x, const int y, const int radius) {
		const int nX = std::max(this->x, std::min(x, this->x + width)),
							nY = std::max(this->y, std::min(y, this->y + height));
		return point_in_circle(nX, nY, x, y, radius);
	}

	bool containsRectangle(const int x, const int y, const int width, const int height) const {
		return (x > this->x && x + width < this->x + this->width &&
				y > this->y && y + height < this->y + this->height);
	}

	bool containsAABB(const AABB &aabb) const {
		return containsRectangle(aabb.x, aabb.y, aabb.width, aabb.height);
	}

	bool intersectsRectangle(const int x, const int y, const int width, const int height) const {
		return (x <= this->x + this->width && x + width >= this->x &&
				y <= this->y + this->height && y + height >= this->y);
	}

	bool intersectsAABB(const AABB &aabb) const {
		return intersectsRectangle(aabb.x, aabb.y, aabb.width, aabb.height);
	}

	friend inline bool operator==(AABB const& lhs, AABB const& rhs)
	{
		return (lhs.x == rhs.x) && (lhs.y == rhs.y) &&
			(lhs.width == rhs.width) && (lhs.height == rhs.height);
	}
};

#endif // AXISALIGNEDBOUNDINGBOX_HPP
