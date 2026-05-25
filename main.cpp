#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile styleFile(":/resources/style.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    } else {
        qWarning() << "Failed to load stylesheet:" << styleFile.fileName();
    }

    MainWindow window;
    window.show();

    return app.exec();
}
