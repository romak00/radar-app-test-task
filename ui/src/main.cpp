#include "window.h"
#include <QApplication>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void detachFromConsole() {
#ifdef _WIN32
    FreeConsole();
#else
    if (!isatty(fileno(stdout))) {
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);
    }
#endif
}

int main(int argc, char** argv) {
    detachFromConsole();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}