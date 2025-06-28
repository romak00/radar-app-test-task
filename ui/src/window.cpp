#include "window.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QPushButton>
#include <QStatusBar>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QCursor>
#include <QMessageBox>
#include <QDir>
#include <QDateTime>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
#ifdef Q_OS_WIN
    setWindowFlags(Qt::FramelessWindowHint);
#else
    setWindowFlags(Qt::Window);
#endif
    setMinimumSize(1024,768);
    resize(1152,864);

    blockMultipleInstances();
    centerWindow();
    setupUI();

    connect(_radar, &RadarWidget::cursorPositionChanged,
        this, [&](double dist, double ang) {
        if (dist <= 1000.0) {
            _cursor_label->setText(
                QString("Cursor: %1 m, %2°")
                .arg(dist, 0, 'f', 1)
                .arg(ang, 0, 'f', 1)
            );
        }
        else {
            _cursor_label->setText("Cursor: —");
        }
    });

    _client = new NetworkClient(this);
    connect(_client, &NetworkClient::newFrame, this, &MainWindow::handleNewFrame);
    connect(_client, &NetworkClient::errorOccured, this, &MainWindow::handleError);
    _client->connectToServer("127.0.0.1", 5555);
}

void MainWindow::setupUI() {
    QWidget* cw = new QWidget;
    auto* hl = new QHBoxLayout(cw);

    _radar = new RadarWidget;
    hl->addWidget(_radar,1);

    QWidget* right = new QWidget;
    right->setFixedWidth(400);
    auto* vl = new QVBoxLayout(right);

    _table = new QTableWidget(0, 3);
    _table->setHorizontalHeaderLabels({ "ID","Distance","Bearing" });
    _table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _table->verticalHeader()->setVisible(false);
    _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _table->setSelectionBehavior(QTableWidget::SelectRows);

    int row_h = _table->verticalHeader()->defaultSectionSize();
    int header_h = _table->horizontalHeader()->height();
    _table->setFixedHeight(header_h + row_h * 9 + 2);

    connect(_radar, &RadarWidget::targetSelected, this, &MainWindow::onTargetSelected);
    connect(_radar, &RadarWidget::cursorPositionChanged, this, &MainWindow::onCursorMoved);

    vl->addWidget(_table);

    connect(_table, &QTableWidget::itemSelectionChanged, this, [this]() {
        auto items = _table->selectedItems();
        if (!items.isEmpty()) {
            int id = items.first()->text().toInt();
            _selected_target_id = id;
            _radar->selectTarget(id);
        }
        else {
            _selected_target_id = -1;
            _radar->selectTarget(-1);
        }
    });

    _cursor_label = new QLabel("Cursor: —");
    _cursor_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    vl->addWidget(_cursor_label);

    _pause_button = new QPushButton("Pause");
    _exit_button  = new QPushButton("Exit");
    connect(_pause_button, &QPushButton::clicked, this, &MainWindow::togglePause);
    connect(_exit_button,  &QPushButton::clicked, this, &MainWindow::exitApp);

    vl->addWidget(_pause_button);
    vl->addWidget(_exit_button);
    vl->addStretch();

    hl->addWidget(right);
    setCentralWidget(cw);

    _status_bar = new QStatusBar(this);
    setStatusBar(_status_bar);

    QTimer* t = new QTimer(this);
    connect(t, &QTimer::timeout, this, &MainWindow::updateTime);
    t->start(1000);
    updateTime();
}

void MainWindow::blockMultipleInstances() {
    QString lock_path = QDir::temp().absoluteFilePath("radar_app.lock");
    _lock_file = new QLockFile(lock_path);
    if (!_lock_file->tryLock(100)) {
        QMessageBox::critical(nullptr, "Error", "Application already running!");
        exit(1);
    }
}

void MainWindow::centerWindow() {
    auto* scr = QGuiApplication::screenAt(QCursor::pos());
    if (!scr) {
        scr = QGuiApplication::primaryScreen();
    }
    QRect g = scr->geometry();
    move(g.x() + (g.width()-width())/2,
         g.y() + (g.height()-height())/2);
}

void MainWindow::togglePause() {
    _paused = !_paused;
    _pause_button->setText(_paused ? "Resume" : "Pause");

    if (_paused) {
        _client->sendCommand("PAUSE");
    }
    else {
        _client->sendCommand("RESUME");
        QTimer::singleShot(0, this, [this]() {
            update();
            });
    }
}

void MainWindow::exitApp() {
    if (_exiting) {
        return;
    }
    _exiting = true;

    setEnabled(false);
    _status_bar->showMessage("Exiting...");

    _client->sendCommand("EXIT");

    QTimer* safetyTimer = new QTimer(this);
    safetyTimer->setSingleShot(true);
    connect(safetyTimer, &QTimer::timeout, this, [this]() {
        qWarning() << "Forced exit after timeout";
        QApplication::quit();
        });
    safetyTimer->start(5000);

    connect(_client, &NetworkClient::errorOccured, this, [this, safetyTimer](const QString&) {
        if (safetyTimer->isActive()) {
            safetyTimer->stop();
            safetyTimer->deleteLater();
        }
        QApplication::quit();
        });
}

void MainWindow::updateTime() {
    _status_bar->showMessage(QTime::currentTime().toString("HH:mm:ss"));
}

void MainWindow::handleNewFrame(const std::vector<Target>& targets) {
    if (_paused) {
        return;
    }
    int current_selection = _selected_target_id;
    _radar->setTargets(targets);

    const int N = int(targets.size());
    int rows = std::max(N, 9);
    _table->setRowCount(rows);

    _table->blockSignals(true);

    _table->clearContents();
    for (int i = 0; i < rows; ++i) {
        if (i < N) {
            const auto& t = targets[i];
            _table->setItem(i, 0, new QTableWidgetItem(QString::number(t.id)));
            _table->setItem(i, 1, new QTableWidgetItem(QString::number(t.distance)));
            double deg = t.angle * 180.0 / EIGEN_PI;
            _table->setItem(i, 2, new QTableWidgetItem(
                QString::number(deg, 'f', 1) + QChar(0x00B0)
            ));

            if (t.id == current_selection) {
                _table->selectRow(i);
            }
        }
        else {
            _table->setItem(i, 0, new QTableWidgetItem(QString()));
            _table->setItem(i, 1, new QTableWidgetItem(QString()));
            _table->setItem(i, 2, new QTableWidgetItem(QString()));
        }
    }

    _table->blockSignals(false);
}

void MainWindow::onTargetSelected(int id) {
    _selected_target_id = id;

    if (id < 0) {
        _table->clearSelection();
        return;
    }

    _table->blockSignals(true);

    for (int i = 0; i < _table->rowCount(); ++i) {
        if (_table->item(i, 0) && _table->item(i, 0)->text().toInt() == id) {
            _table->selectRow(i);
            QModelIndex idx = _table->model()->index(i, 0);
            _table->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            break;
        }
    }

    _table->blockSignals(false);
}

void MainWindow::onCursorMoved(double dist, double ang) {
    _status_bar->showMessage(QString("Cursor: %1 m, %2°").arg(dist).arg(ang));
}

void MainWindow::handleError(const QString& msg) {
    if (_exiting) {
        return;
    }
    QMessageBox::warning(this, "Network Error", msg);
}

#ifdef Q_OS_WIN
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, long* result) {
    MSG* msg = static_cast<MSG*>(message);

    if (msg->message == WM_NCHITTEST)
    {
        if (isMaximized())
        {
            return false;
        }

        *result = 0;
        const LONG borderWidth = 8;
        RECT winrect;
        GetWindowRect(reinterpret_cast<HWND>(winId()), &winrect);

        short x = msg->lParam & 0x0000FFFF;
        short y = (msg->lParam & 0xFFFF0000) >> 16;

        bool resizeWidth = minimumWidth() != maximumWidth();
        bool resizeHeight = minimumHeight() != maximumHeight();
        if (resizeWidth)
        {
            if (x >= winrect.left && x < winrect.left + borderWidth)
            {
                *result = HTLEFT;
            }
            if (x < winrect.right && x >= winrect.right - borderWidth)
            {
                *result = HTRIGHT;
            }
        }
        if (resizeHeight)
        {
            if (y < winrect.bottom && y >= winrect.bottom - borderWidth)
            {
                *result = HTBOTTOM;
            }
            if (y >= winrect.top && y < winrect.top + borderWidth)
            {
                *result = HTTOP;
            }
        }
        if (resizeWidth && resizeHeight)
        {
            if (x >= winrect.left && x < winrect.left + borderWidth &&
                y < winrect.bottom && y >= winrect.bottom - borderWidth)
            {
                *result = HTBOTTOMLEFT;
            }
            if (x < winrect.right && x >= winrect.right - borderWidth &&
                y < winrect.bottom && y >= winrect.bottom - borderWidth)
            {
                *result = HTBOTTOMRIGHT;
            }
            if (x >= winrect.left && x < winrect.left + borderWidth &&
                y >= winrect.top && y < winrect.top + borderWidth)
            {
                *result = HTTOPLEFT;
            }
            if (x < winrect.right && x >= winrect.right - borderWidth &&
                y >= winrect.top && y < winrect.top + borderWidth)
            {
                *result = HTTOPRIGHT;
            }
        }

        if (*result != 0)
            return true;

        QWidget* action = QApplication::widgetAt(QCursor::pos());
        if (action == this) {
            *result = HTCAPTION;
            return true;
        }
    }

    return false;
}
#endif