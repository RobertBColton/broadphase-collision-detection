#include "MainWindow.hpp"
#include "Broadphase.hpp"
#include "Quadtree.hpp"
#include "SpatialHash.hpp"
#include "PruneSweep.hpp"

#include <QtWidgets>

#include <algorithm>
#include <cstdlib>

static size_t allocated_bytes = 0;

void * operator new(size_t size)
{
	allocated_bytes += size;
	void * p = std::malloc(size);
	return p;
}

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

using CreateRandom = std::function<std::vector<AABB>(void)>;

QTableWidgetItem* createTimeCellItem(double time) {
	auto item = new QTableWidgetItem();
	item->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
	item->setText(QString::number(time, 'f', 2));
	return item;
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

	allocated_bytes = 0;
	auto quadtree = new Quadtree();
	const size_t quadtreeSize = allocated_bytes;
	allocated_bytes = 0;
	auto spatialHash = new SpatialHash();
	const size_t spatialHashSize = allocated_bytes;
	allocated_bytes = 0;
	auto pruneSweep = new PruneSweep();
	const size_t pruneSweepSize = allocated_bytes;

	QList<QPair<QString, QSharedPointer<Broadphase>>> bpis = {
		{"Prune Sweep",QSharedPointer<Broadphase>(pruneSweep)},
		{"Quadtree",QSharedPointer<Broadphase>(quadtree)},
		{"Spatial Hash",QSharedPointer<Broadphase>(spatialHash)},
	};
	QList<size_t> base_sizes = { pruneSweepSize, quadtreeSize, spatialHashSize };

	auto createRandomDense = []() {
		std::vector<AABB> aabbs;
		for (int i = 0; i < 10000; ++i) {
			const int width = randomInt(4, 32),
								height = randomInt(4, 32);
			const auto aabb = AABB(randomInt(-width, 1024),
														 randomInt(-height, 1024),
														 width,
														 height);
			aabbs.emplace_back(aabb);
		}
		return aabbs;
	};

	auto createRandomSparse = [](){
		std::vector<AABB> aabbs;
		for (int c = 0; c < 10; ++c) {
			int cx = randomInt(0, 1024);
			int cy = randomInt(0, 1024);
			for (int i = 0; i < 1000; ++i) {
				const int width = randomInt(4, 32),
									height = randomInt(4, 32);
				const auto aabb = AABB(cx + randomInt(-std::min(32, width), 0),
															 cy + randomInt(-std::min(32, height), 0),
															 width, height);
				aabbs.emplace_back(aabb);
			}
		}
		return aabbs;
	};

	bmButton->connect(bmButton, &QAbstractButton::clicked, [&](){
		QDialog benchmarkWindow(nullptr);
		benchmarkWindow.setWindowFlags(launcher.windowFlags() & ~Qt::WindowContextHelpButtonHint);
		benchmarkWindow.setWindowTitle("Benchmark");

		QTableWidget* benchmarkTable = new QTableWidget(bpis.size() * 2 + 2, 6);
		benchmarkTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		benchmarkTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
		benchmarkTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
		benchmarkTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

		benchmarkTable->setVerticalHeaderItem(0, new QTableWidgetItem("Dense"));
		benchmarkTable->setVerticalHeaderItem(bpis.size() + 1, new QTableWidgetItem("Sparse"));
		benchmarkTable->setSpan(0,0,1,6);
		benchmarkTable->setSpan(bpis.size() + 1,0,1,6);

		for (int i = 0; i < bpis.size(); ++i) {
			auto bpi = bpis[i];
			benchmarkTable->setVerticalHeaderItem(i + 1, new QTableWidgetItem(bpi.first));
			benchmarkTable->setVerticalHeaderItem(bpis.size() + i + 2, new QTableWidgetItem(bpi.first));

			auto benchmarkBroadphase =
				[&](CreateRandom createRandom, int row) {
					std::vector<Broadphase::Proxy*> proxies;
					std::vector<AABB> aabbs = createRandom();
					size_t memory = 0;
					double insert = benchmark(
						[&](bool, bool finalRun){
							allocated_bytes = 0;
							for (const auto& aabb : aabbs) {
								const auto proxy = bpi.second->addProxy(aabb);
								if (finalRun) proxies.push_back(proxy);
							}
							memory = allocated_bytes;
						},
						[&](bool, bool){
							bpi.second->clear();
						}
					);
					double query = benchmark([=](bool, bool) {
						for (int i = 0; i < 100; ++i) {
							bpi.second->queryRange(randomInt(0, 1024),
																		 randomInt(0, 1024),
																		 randomInt(2, 240));
						}
					});
					double update = benchmark([=](bool, bool) {
						size_t aabbId = aabbs.size();
						for (auto proxy : proxies) {
							bpi.second->updateProxy(proxy, aabbs[--aabbId]);
						}
					});
					double clear = benchmark(
						[=](bool, bool){
							bpi.second->clear();
						},
						[](bool, bool){},
						[&](bool, bool finalRun){
							if (!finalRun) {
								for (const auto& aabb : aabbs)
									bpi.second->addProxy(aabb);
							}
						}
					);
					double remove = benchmark(
						[&](bool, bool){
							for (auto proxy : proxies)
								bpi.second->removeProxy(proxy);
						},
						[&](bool, bool){
							std::vector<Broadphase::Proxy*>().swap(proxies);
							for (const auto& aabb : aabbs)
								proxies.push_back(bpi.second->addProxy(aabb));
						}
					);

					memory += base_sizes[i];
					auto memoryItem = new QTableWidgetItem(benchmarkTable->locale().formattedDataSize(memory));
					memoryItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
					benchmarkTable->setItem(row, 0, memoryItem);
					benchmarkTable->setItem(row, 1, createTimeCellItem(insert));
					benchmarkTable->setItem(row, 2, createTimeCellItem(query));
					benchmarkTable->setItem(row, 3, createTimeCellItem(update));
					benchmarkTable->setItem(row, 4, createTimeCellItem(clear));
					benchmarkTable->setItem(row, 5, createTimeCellItem(remove));
				};

			benchmarkBroadphase(createRandomDense, i + 1);
			benchmarkBroadphase(createRandomSparse, bpis.size() + i + 2);
		}

		//benchmarkTable->setSortingEnabled(true);
		benchmarkTable->setHorizontalHeaderLabels({"Memory", "Insert", "Query", "Update", "Clear", "Remove"});

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
