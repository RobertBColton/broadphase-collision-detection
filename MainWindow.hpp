#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include "Broadphase.hpp"

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
	Broadphase *broadphase;
	std::vector<QGraphicsRectItem*> objects;
	QGraphicsEllipseItem *player;
	qreal playerRadius = 50.0f;
	QGraphicsView *view;
};

#endif // MAINWINDOW_HPP
