#include "qtclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtClient qtc(nullptr, 1739, 1732);
    QObject::connect(&qtc, SIGNAL(finished()), &a, SLOT(quit()));
    qtc.bind();
    qtc.show();
    return a.exec();
}
