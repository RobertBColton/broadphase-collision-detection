#ifndef BROADPHASE_H
#define BROADPHASE_H

class Broadphase {
	Broadphase();
public:
	virtual ~Broadphase() {}
	virtual void add() = 0;
	virtual void remove() = 0;
	virtual void queryRange() = 0;
};

#endif // BROADPHASE_H
