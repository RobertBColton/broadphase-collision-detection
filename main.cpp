#include "MainWindow.hpp"
#include "Broadphase.hpp"
#include "Quadtree.hpp"
#include "SpatialHash.hpp"

#include <QtWidgets>

#include <algorithm>

static int randomInt(int low, int high) {
	return rand() % ((high + 1) - low) + low;
}

double benchmark(std::function<void(bool,bool)> fnc,
								 std::function<void(bool,bool)> pre = [](bool,bool){},
								 std::function<void(bool,bool)> post = [](bool,bool){},
								 int runs = 5) {
	double avgTime = 0;
	for (int i = 0; i < runs;) {
		bool firstRun = (i == 0),
				 finalRun = (i == runs - 1);
		srand(1);
		pre(firstRun, finalRun);

		auto start = std::chrono::high_resolution_clock::now();
		fnc(firstRun, finalRun);
		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = end - start;
		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		double delta = microseconds - avgTime;
		avgTime += delta/++i;

		post(firstRun, finalRun);
	}
	return avgTime;
}

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

	std::function<std::vector<Broadphase::Proxy*>(Broadphase*)> createRandomDense = [=](Broadphase* bp){
		std::vector<Broadphase::Proxy*> proxies;
		for (int i = 0; i < 10000; ++i) {
			const int width = randomInt(4, 320),
								height = randomInt(4, 320);
			auto proxy = bp->addProxy(
				AABB(randomInt(-width, 1024),
						 randomInt(-height, 1024),
						 width,
						 height)
			);
			proxies.push_back(proxy);
		}
		return proxies;
	};

	std::function<std::vector<Broadphase::Proxy*>(Broadphase*)> createRandomSparse = [=](Broadphase* bp){
		std::vector<Broadphase::Proxy*> proxies;
		for (int c = 0; c < 10; ++c) {
			int cx = randomInt(0, 1024);
			int cy = randomInt(0, 1024);
			for (int i = 0; i < 1000; ++i) {
				const int width = randomInt(4, 128),
									height = randomInt(4, 128);
				auto proxy = bp->addProxy(
					AABB(cx + randomInt(-std::min(128, width), 0),
							 cy + randomInt(-std::min(128, height), 0),
							 width,
							 height)
				);
				proxies.push_back(proxy);
			}
		}
		return proxies;
	};

	bmButton->connect(bmButton, &QAbstractButton::clicked, [&](){
		QDialog benchmarkWindow(nullptr);
		benchmarkWindow.setWindowFlags(launcher.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		benchmarkWindow.setWindowTitle("Benchmark");

		QTableWidget* benchmarkTable = new QTableWidget(bpis.size() * 2 + 2, 5);
		benchmarkTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		benchmarkTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
		benchmarkTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
		benchmarkTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

		benchmarkTable->setVerticalHeaderItem(0, new QTableWidgetItem("Dense"));
		benchmarkTable->setVerticalHeaderItem(bpis.size() + 1, new QTableWidgetItem("Sparse"));
		benchmarkTable->setSpan(0,0,1,5);
		benchmarkTable->setSpan(bpis.size() + 1,0,1,5);

		for (int i = 0; i < bpis.size(); ++i) {
			auto bpi = bpis[i];
			benchmarkTable->setVerticalHeaderItem(i + 1, new QTableWidgetItem(bpi.first));
			benchmarkTable->setVerticalHeaderItem(bpis.size() + i + 2, new QTableWidgetItem(bpi.first));

			auto benchmarkBroadphase = [=](std::function<std::vector<Broadphase::Proxy*>(Broadphase*)> createRandom, int row) {
				double insert = benchmark(
					[=](bool, bool){
						createRandom(bpi.second.get());
					},
					[=](bool, bool){
						bpi.second->clear();
					}
				);
				size_t memory = sizeof(bpi.second);
				double query = benchmark([=](bool, bool) {
					for (int i = 0; i < 100; ++i) {
						bpi.second->queryRange(randomInt(0, 1024),
																	 randomInt(0, 1024),
																	 randomInt(2, 240));
					}
				});
				double clear = benchmark(
					[=](bool, bool){
						bpi.second->clear();
					},
					[](bool,bool){},
					[&](bool,bool finalRun){
						if (!finalRun) createRandom(bpi.second.get());
					}
				);
				std::vector<Broadphase::Proxy*> proxies;
				double remove = benchmark(
					[&](bool, bool){
						for (auto proxy : proxies)
							bpi.second->removeProxy(proxy);
					},
					[&](bool,bool){
						proxies = createRandom(bpi.second.get());
					}
				);

				benchmarkTable->setItem(row, 0, new QTableWidgetItem(QString::number(memory)));
				benchmarkTable->setItem(row, 1, new QTableWidgetItem(QString::number(insert)));
				benchmarkTable->setItem(row, 2, new QTableWidgetItem(QString::number(query)));
				benchmarkTable->setItem(row, 3, new QTableWidgetItem(QString::number(clear)));
				benchmarkTable->setItem(row, 4, new QTableWidgetItem(QString::number(remove)));
			};

			benchmarkBroadphase(createRandomDense, i + 1);
			benchmarkBroadphase(createRandomSparse, bpis.size() + i + 2);
		}

		//benchmarkTable->setSortingEnabled(true);
		benchmarkTable->setHorizontalHeaderLabels({"Memory", "Insert", "Query", "Clear", "Remove"});

		QVBoxLayout* bmlayout = new QVBoxLayout();
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
