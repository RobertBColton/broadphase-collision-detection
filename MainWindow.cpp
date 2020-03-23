#include "MainWindow.hpp"
#include "Broadphase.hpp"

static int randomInt(int low, int high) {
	return qrand() % ((high + 1) - low) + low;
}

MainWindow::MainWindow(Broadphase *broadphase, QWidget *parent) :
	QMainWindow(parent), broadphase(broadphase)
{
	QTime time = QTime::currentTime();
	qsrand((uint)time.msec());

	scene = new QGraphicsScene(0, 0, 1024, 1024, this);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);

	// create the player
	player = scene->addEllipse(
				0, 0, playerRadius*2, playerRadius*2, QPen(Qt::black), Qt::green);

	// create some random objects
	for (size_t i = 0; i < 10000; ++i) {
		const int width = randomInt(10, 25),
							height = randomInt(10, 25);
		QGraphicsRectItem* object = scene->addRect(0,0,width,height);
		object->setPos(
			randomInt(-width, scene->width()),
			randomInt(-height, scene->height())
		);
		object->setData(0, QPointF(randomInt(-2, 2), randomInt(-2, 2)));

		// add it to the broadphase
		auto proxy = broadphase->addProxy(
			AABB(object->x(),object->y(),width,height),
			object
		);

		proxies.push_back(proxy);
	}

	// create a background grid that shows the buckets
	QPen gridPen(Qt::black, 5);
	qreal cellWidth = 64;// broadphase.getCellWidth();
	qreal cellHeight = 64;// broadphase.getCellHeight();
	for (qreal x = 0; x <= scene->width(); x += cellWidth)
		scene->addLine(x, 0, x, scene->height(), gridPen);
	for (qreal y = 0; y <= scene->height(); y += cellHeight)
		scene->addLine(0, y, scene->width(), y, gridPen);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateGame()));
	timer->start(0);

	view = new QGraphicsView(scene, this);
	view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);

	this->setCentralWidget(view);
	this->setWindowTitle("Broadphase Collision Detection");
}

MainWindow::~MainWindow()
{
	broadphase->clear();
}

void MainWindow::updateGame() {
	for (auto proxy : proxies) {
		auto object = (QGraphicsRectItem*)proxy->userdata;

		// move the object around
		QPointF velocity = object->data(0).toPointF();
		auto aabb = proxy->aabb;
		aabb.setPosition(aabb.getX() + velocity.x(),
										 aabb.getY() + velocity.y());
		aabb.warp(AABB(0, 0, 1024, 1024));
		object->setPos(aabb.getX(), aabb.getY());

		// reset its color until we see if it collided with the player
		object->setBrush(Qt::darkCyan);
		object->setPen(QPen(Qt::black));

		broadphase->updateProxy(proxy, aabb);
	}

	// now query the cursor and change the color of objects
	// that hit the player to red
	player->setPos(view->mapFromGlobal(QCursor::pos() - QPoint(playerRadius, playerRadius)));
	const auto &hits = broadphase->queryRange(
				player->x() + playerRadius,
				player->y() + playerRadius,
				playerRadius);

	for (const auto &hit : hits) {
		auto other = (QAbstractGraphicsShapeItem*)hit->userdata;
		if (other == nullptr) continue;
		other->setBrush(Qt::red);
	}

	// do the repainting manually
	view->viewport()->update();
}
