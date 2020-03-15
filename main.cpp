#include "MainWindow.hpp"
#include "Broadphase.h"
#include "Quadtree.h"

#include <QtWidgets>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QDialog launcher(nullptr);
	launcher.setWindowFlags(launcher.windowFlags() & ~Qt::WindowContextHelpButtonHint);
	launcher.setWindowTitle("Broadphase Collision Detection");

	QVBoxLayout *vbl = new QVBoxLayout();
	QPushButton *bmButton = new QPushButton("Benchmark");
	vbl->addWidget(bmButton);

	QList<QPair<QString, QSharedPointer<Broadphase>>> bpis = {
		{"K-D Tree",QSharedPointer<Broadphase>(new Quadtree())},
		{"K-D-B Tree",QSharedPointer<Broadphase>(new Quadtree())},
		{"PR Quadtree",QSharedPointer<Broadphase>(new Quadtree())},
		{"Quadtree",QSharedPointer<Broadphase>(new Quadtree())},
	};
	foreach (auto bp, bpis) {
		QPushButton *bpButton = new QPushButton(bp.first);
		vbl->addWidget(bpButton);
	}

	launcher.setLayout(vbl);
	launcher.resize(600,400);
	launcher.show();

	//MainWindow w;
	//w.show();

	return a.exec();
}
