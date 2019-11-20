#include "qtclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtClient qtc(nullptr, 1739, 1732);
    QObject::connect(&qtc, SIGNAL(finished()), &a, SLOT(quit()));
    // binds to listen udp datagrams at port 1732
    // as soon as data is flowing in
    // a signal is emitted
    // and a function handling that signal
    // the handling function measures the length of the incoming data
    // and saves it to a datagram property
    // after successful reading of a datagram
    qtc.bind();
    qtc.show();
    return a.exec();
}
