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

	QGraphicsScene *scene = new QGraphicsScene(0, 0, 1024, 1024, this);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);

	// create the player
	player = scene->addEllipse(
				0, 0, playerRadius*2, playerRadius*2, QPen(Qt::black), Qt::green);

	// create some random objects
	for (size_t i = 0; i < 10000; ++i) {
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

}

void MainWindow::updateGame() {

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
		broadphase->addProxy(
			AABB(object->x(),object->y(),
					 object->boundingRect().width(),object->boundingRect().height()),
			object);
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

	// clear broadphase for the next run
	broadphase->clear();

	// do the repainting manually
	view->viewport()->update();
}
