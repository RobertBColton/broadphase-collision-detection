#include "Quadtree.h"

Quadtree::Quadtree(int width, int height, int depth): Broadphase(), root(AABB(width, height)), depth(depth)
{
	buildTree(&root);
}

void Quadtree::buildTree(Node* root, int cd) {
	if (cd >= depth) return;
	int newWidth = root->aabb.getWidth() / 2,
			newHeight = root->aabb.getHeight() / 2;
	root->NW = new Node(AABB(root->aabb.getX(), root->aabb.getY(), newWidth, newHeight));
	root->NE = new Node(AABB(root->aabb.getX() + newWidth, root->aabb.getY(), newWidth, newHeight));
	root->SW = new Node(AABB(root->aabb.getX(), root->aabb.getY() + newHeight, newWidth, newHeight));
	root->SE = new Node(AABB(root->aabb.getX() + newWidth, root->aabb.getY() + newHeight, newWidth, newHeight));
	buildTree(root->NW, cd + 1);
	buildTree(root->NE, cd + 1);
	buildTree(root->SW, cd + 1);
	buildTree(root->SE, cd + 1);
}

Broadphase::Proxy* Quadtree::addProxy(AABB aabb, void* userdata) {
	Proxy* proxy = new Proxy(aabb, userdata);
	auto ret = root.addProxy(proxy);
	if (!ret) delete proxy;
	return ret;
}

void Quadtree::removeProxy(Proxy* proxy) {
	root.removeProxy(proxy);
	delete proxy;
}

void Quadtree::clear() {
	root.clear();
}

std::unordered_set<Broadphase::Proxy*> Quadtree::queryRange(const int x, const int y, const int radius) {
	std::unordered_set<Broadphase::Proxy*> hits;
	root.queryRange(x, y, radius, hits);
	return hits;
}
