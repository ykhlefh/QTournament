/****************************************************************************
** Meta object code from reading C++ file 'PlayerMngr.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "PlayerMngr.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlayerMngr.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QTournament__PlayerMngr_t {
    QByteArrayData data[8];
    char stringdata[98];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QTournament__PlayerMngr_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QTournament__PlayerMngr_t qt_meta_stringdata_QTournament__PlayerMngr = {
    {
QT_MOC_LITERAL(0, 0, 23),
QT_MOC_LITERAL(1, 24, 17),
QT_MOC_LITERAL(2, 42, 0),
QT_MOC_LITERAL(3, 43, 15),
QT_MOC_LITERAL(4, 59, 15),
QT_MOC_LITERAL(5, 75, 13),
QT_MOC_LITERAL(6, 89, 6),
QT_MOC_LITERAL(7, 96, 1)
    },
    "QTournament::PlayerMngr\0beginCreatePlayer\0"
    "\0endCreatePlayer\0newPlayerSeqNum\0"
    "playerRenamed\0Player\0p"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QTournament__PlayerMngr[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x06 /* Public */,
       3,    1,   30,    2, 0x06 /* Public */,
       5,    1,   33,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, 0x80000000 | 6,    7,

       0        // eod
};

void QTournament::PlayerMngr::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PlayerMngr *_t = static_cast<PlayerMngr *>(_o);
        switch (_id) {
        case 0: _t->beginCreatePlayer(); break;
        case 1: _t->endCreatePlayer((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->playerRenamed((*reinterpret_cast< const Player(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (PlayerMngr::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PlayerMngr::beginCreatePlayer)) {
                *result = 0;
            }
        }
        {
            typedef void (PlayerMngr::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PlayerMngr::endCreatePlayer)) {
                *result = 1;
            }
        }
        {
            typedef void (PlayerMngr::*_t)(const Player & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PlayerMngr::playerRenamed)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject QTournament::PlayerMngr::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QTournament__PlayerMngr.data,
      qt_meta_data_QTournament__PlayerMngr,  qt_static_metacall, 0, 0}
};


const QMetaObject *QTournament::PlayerMngr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QTournament::PlayerMngr::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QTournament__PlayerMngr.stringdata))
        return static_cast<void*>(const_cast< PlayerMngr*>(this));
    if (!strcmp(_clname, "GenericObjectManager"))
        return static_cast< GenericObjectManager*>(const_cast< PlayerMngr*>(this));
    return QObject::qt_metacast(_clname);
}

int QTournament::PlayerMngr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void QTournament::PlayerMngr::beginCreatePlayer()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void QTournament::PlayerMngr::endCreatePlayer(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QTournament::PlayerMngr::playerRenamed(const Player & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
