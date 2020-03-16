#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Broadphase.h"

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
};

#endif // MAINWINDOW_HPP
