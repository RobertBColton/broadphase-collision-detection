#include "MainWindow.hpp"
#include "Broadphase.h"
#include "Quadtree.h"
#include "SpatialHash.hpp"

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
		{"Quadtree",QSharedPointer<Broadphase>(new Quadtree())},
		{"Spatial Hash",QSharedPointer<Broadphase>(new SpatialHash())},
	};

	bmButton->connect(bmButton, &QAbstractButton::clicked, [&](){
		QDialog benchmarkWindow(nullptr);
		benchmarkWindow.setWindowFlags(launcher.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		benchmarkWindow.setWindowTitle("Benchmark");

		QVBoxLayout* bmlayout = new QVBoxLayout();
		QTableWidget* benchmarkTable = new QTableWidget(bpis.size(), 4);
		benchmarkTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		benchmarkTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		benchmarkTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
		benchmarkTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
		benchmarkTable->setHorizontalHeaderLabels({"Memory", "Insert", "Query", "Delete"});
		bmlayout->addWidget(benchmarkTable);
		benchmarkWindow.setLayout(bmlayout);

		benchmarkWindow.exec();
	});

	foreach (auto bp, bpis) {
		QPushButton *bpButton = new QPushButton(bp.first);
		bpButton->connect(bpButton, &QAbstractButton::clicked, [=](){
			auto w = new MainWindow(bp.second.get());
			w->setWindowTitle(bp.first);
			w->setAttribute(Qt::WA_ShowModal, true);
			w->setAttribute(Qt::WA_DeleteOnClose, true);
			w->show();
		});
		vbl->addWidget(bpButton);
	}

	launcher.setLayout(vbl);
	launcher.resize(600,400);
	launcher.show();

	return a.exec();
}
