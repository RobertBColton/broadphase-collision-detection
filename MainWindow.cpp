#include "MainWindow.hpp"
#include "SparseSpatialBroadphase.hpp"

#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QTimer>
#include <QTime>

#include <vector>

SparseSpatialBroadphase broadphase(100, 100);

std::vector<QGraphicsRectItem*> objects;
QGraphicsRectItem* player;
QGraphicsView *view;

int randomInt(int low, int high) {
	return qrand() % ((high + 1) - low) + low;
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	QTime time = QTime::currentTime();
	qsrand((uint)time.msec());

	QGraphicsScene *scene = new QGraphicsScene(0, 0, 1200, 720, this);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);

	// create the player
	player = scene->addRect(0, 0, 50, 50, QPen(Qt::black), Qt::green);

	// create some random objects
	for (size_t i = 0; i < 5000; ++i) {
		QGraphicsRectItem* object = scene->addRect(
			0, 0, randomInt(10, 25), randomInt(10, 25));
		object->setPos(
			randomInt(-object->boundingRect().width(), scene->width()),
			randomInt(-object->boundingRect().height(), scene->height()));
		object->setData(0, QPointF(randomInt(-2, 2), randomInt(-2, 2)));
		objects.push_back(object);
	}

	// create a background grid that shows the buckets
	QPen gridPen(Qt::black, 5);
	for (int x = 0; x <= scene->width(); x += broadphase.getCellWidth())
		scene->addLine(x, 0, x, scene->height(), gridPen);
	for (int y = 0; y <= scene->height(); y += broadphase.getCellHeight())
		scene->addLine(0, y, scene->width(), y, gridPen);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateGame()));
	timer->start(0);

	view = new QGraphicsView(scene);
	view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);

	this->setCentralWidget(view);
	this->setWindowTitle("Broadphase Collision Detection");
}

MainWindow::~MainWindow()
{

}

void MainWindow::updateGame() {
	player->setPos(view->mapFromGlobal(QCursor::pos() - QPoint(25, 25)));

	// add the player to the broadphase
	broadphase.addRectangle(
		player->x(),
		player->y(),
		player->boundingRect().width(),
		player->boundingRect().height(),
		player);

	for (const auto& object : objects) {
		// move the object around
		QPointF velocity = object->data(0).toPointF();
		object->moveBy(velocity.x(), velocity.y());
		if (object->x() < -object->boundingRect().width()) {
			object->setX(this->width());
		} else if (object->x() > this->width()) {
			object->setX(-object->boundingRect().width());
		}
		if (object->y() < -object->boundingRect().height()) {
			object->setY(this->height());
		} else if (object->y() > this->height()) {
			object->setY(-object->boundingRect().height());
		}

		// reset its color until we see if it collided with the player
		object->setBrush(Qt::darkCyan);
		object->setPen(QPen(Qt::black));

		// add it to the broadphase
		broadphase.addRectangle(
			object->x(),
			object->y(),
			object->boundingRect().width(),
			object->boundingRect().height(),
			object);
	}

	// now query the collision pairs and change the color of objects that hit the player to red
	const auto &collisionPairs = broadphase.getCollisionPairs();

	for (const auto &pair : collisionPairs) {
		QGraphicsRectItem *other = nullptr;
		if (pair.first == player) {
			other = (QGraphicsRectItem*)pair.second;
		} else if (pair.second == player) {
			other = (QGraphicsRectItem*)pair.first;
		} else {
			((QGraphicsRectItem*)pair.first)->setPen(QPen(Qt::cyan));
			 ((QGraphicsRectItem*)pair.second)->setPen(QPen(Qt::cyan));
		}
		if (other != nullptr)
			other->setBrush(Qt::red);
	}

	// clear broadphase for the next run
	broadphase.clear();

	// do the repainting manually
	view->viewport()->update();
}
