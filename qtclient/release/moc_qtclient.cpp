/****************************************************************************
** Meta object code from reading C++ file 'qtclient.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.13.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../qtclient.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.13.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtClient_t {
    QByteArrayData data[21];
    char stringdata0[360];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtClient_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtClient_t qt_meta_stringdata_QtClient = {
    {
QT_MOC_LITERAL(0, 0, 8), // "QtClient"
QT_MOC_LITERAL(1, 9, 8), // "finished"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 18), // "slot_readyRead_udp"
QT_MOC_LITERAL(4, 38, 14), // "slot_hostFound"
QT_MOC_LITERAL(5, 53, 14), // "slot_connected"
QT_MOC_LITERAL(6, 68, 17), // "slot_disconnected"
QT_MOC_LITERAL(7, 86, 18), // "slot_readyRead_tcp"
QT_MOC_LITERAL(8, 105, 12), // "slot_timeout"
QT_MOC_LITERAL(9, 118, 16), // "on_pbSSM_clicked"
QT_MOC_LITERAL(10, 135, 16), // "on_pbSHM_clicked"
QT_MOC_LITERAL(11, 152, 16), // "on_pbUFM_clicked"
QT_MOC_LITERAL(12, 169, 16), // "on_pbRLM_clicked"
QT_MOC_LITERAL(13, 186, 16), // "on_pbPOM_clicked"
QT_MOC_LITERAL(14, 203, 16), // "on_pbMCM_clicked"
QT_MOC_LITERAL(15, 220, 22), // "on_pbSBMfreeze_clicked"
QT_MOC_LITERAL(16, 243, 21), // "on_read_sbm_triggered"
QT_MOC_LITERAL(17, 265, 28), // "on_connect_to_host_triggered"
QT_MOC_LITERAL(18, 294, 21), // "on_send_ncm_triggered"
QT_MOC_LITERAL(19, 316, 21), // "on_send_ccm_triggered"
QT_MOC_LITERAL(20, 338, 21) // "on_send_sqm_triggered"

    },
    "QtClient\0finished\0\0slot_readyRead_udp\0"
    "slot_hostFound\0slot_connected\0"
    "slot_disconnected\0slot_readyRead_tcp\0"
    "slot_timeout\0on_pbSSM_clicked\0"
    "on_pbSHM_clicked\0on_pbUFM_clicked\0"
    "on_pbRLM_clicked\0on_pbPOM_clicked\0"
    "on_pbMCM_clicked\0on_pbSBMfreeze_clicked\0"
    "on_read_sbm_triggered\0"
    "on_connect_to_host_triggered\0"
    "on_send_ncm_triggered\0on_send_ccm_triggered\0"
    "on_send_sqm_triggered"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtClient[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  109,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,  110,    2, 0x0a /* Public */,
       4,    0,  111,    2, 0x0a /* Public */,
       5,    0,  112,    2, 0x0a /* Public */,
       6,    0,  113,    2, 0x0a /* Public */,
       7,    0,  114,    2, 0x0a /* Public */,
       8,    0,  115,    2, 0x0a /* Public */,
       9,    0,  116,    2, 0x08 /* Private */,
      10,    0,  117,    2, 0x08 /* Private */,
      11,    0,  118,    2, 0x08 /* Private */,
      12,    0,  119,    2, 0x08 /* Private */,
      13,    0,  120,    2, 0x08 /* Private */,
      14,    0,  121,    2, 0x08 /* Private */,
      15,    0,  122,    2, 0x08 /* Private */,
      16,    0,  123,    2, 0x08 /* Private */,
      17,    0,  124,    2, 0x08 /* Private */,
      18,    0,  125,    2, 0x08 /* Private */,
      19,    0,  126,    2, 0x08 /* Private */,
      20,    0,  127,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void QtClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QtClient *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->finished(); break;
        case 1: _t->slot_readyRead_udp(); break;
        case 2: _t->slot_hostFound(); break;
        case 3: _t->slot_connected(); break;
        case 4: _t->slot_disconnected(); break;
        case 5: _t->slot_readyRead_tcp(); break;
        case 6: _t->slot_timeout(); break;
        case 7: _t->on_pbSSM_clicked(); break;
        case 8: _t->on_pbSHM_clicked(); break;
        case 9: _t->on_pbUFM_clicked(); break;
        case 10: _t->on_pbRLM_clicked(); break;
        case 11: _t->on_pbPOM_clicked(); break;
        case 12: _t->on_pbMCM_clicked(); break;
        case 13: _t->on_pbSBMfreeze_clicked(); break;
        case 14: _t->on_read_sbm_triggered(); break;
        case 15: _t->on_connect_to_host_triggered(); break;
        case 16: _t->on_send_ncm_triggered(); break;
        case 17: _t->on_send_ccm_triggered(); break;
        case 18: _t->on_send_sqm_triggered(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (QtClient::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QtClient::finished)) {
                *result = 0;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject QtClient::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_QtClient.data,
    qt_meta_data_QtClient,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QtClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtClient.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int QtClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void QtClient::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
