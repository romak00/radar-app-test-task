#pragma once

#include <QMainWindow>
#include <QLockFile>
#include <QLabel>
#include "radar-widget.h"
#include "network-client.h"

class QTableWidget;
class QPushButton;
class QStatusBar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);

private slots:
    void togglePause();
    void exitApp();
    void updateTime();
    void handleNewFrame(const std::vector<Target>& targets);
    void onTargetSelected(int id);
    void onCursorMoved(double dist, double angle);
    void handleError(const QString& msg);

private:
    void setupUI();
    void centerWindow();
    void blockMultipleInstances();

    RadarWidget* _radar;
    QTableWidget* _table;
    QPushButton* _pause_button;
    QPushButton* _exit_button;
    QStatusBar* _status_bar;
    NetworkClient* _client;
    QLockFile* _lock_file;
    QLabel* _cursor_label;
    int _selected_target_id = -1;
    bool _paused = false;
    bool _exiting = false;
};
