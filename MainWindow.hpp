#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "Broadphase.hpp"

#include <QtWidgets>

#include <vector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(Broadphase *broadphase = 0, QWidget *parent = 0);
	~MainWindow();

public slots:
	void updateGame();

private:
	QGraphicsScene* scene;
	Broadphase *broadphase;
	std::vector<Broadphase::Proxy*> proxies;
	QGraphicsEllipseItem *player;
	qreal playerRadius = 50.0f;
	QGraphicsView *view;
};

#endif // MAINWINDOW_HPP
