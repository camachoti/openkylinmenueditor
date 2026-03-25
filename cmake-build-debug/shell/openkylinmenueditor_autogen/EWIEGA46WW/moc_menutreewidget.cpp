/****************************************************************************
** Meta object code from reading C++ file 'menutreewidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.10)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../shell/menutreewidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'menutreewidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.10. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MenuTreeWidget_t {
    QByteArrayData data[17];
    char stringdata0[237];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MenuTreeWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MenuTreeWidget_t qt_meta_stringdata_MenuTreeWidget = {
    {
QT_MOC_LITERAL(0, 0, 14), // "MenuTreeWidget"
QT_MOC_LITERAL(1, 15, 12), // "itemSelected"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 8), // "filename"
QT_MOC_LITERAL(4, 38, 8), // "itemType"
QT_MOC_LITERAL(5, 47, 20), // "addLauncherRequested"
QT_MOC_LITERAL(6, 68, 21), // "addDirectoryRequested"
QT_MOC_LITERAL(7, 90, 21), // "addSeparatorRequested"
QT_MOC_LITERAL(8, 112, 15), // "removeRequested"
QT_MOC_LITERAL(9, 128, 15), // "moveUpRequested"
QT_MOC_LITERAL(10, 144, 17), // "moveDownRequested"
QT_MOC_LITERAL(11, 162, 14), // "reloadRequired"
QT_MOC_LITERAL(12, 177, 15), // "onSearchChanged"
QT_MOC_LITERAL(13, 193, 4), // "text"
QT_MOC_LITERAL(14, 198, 18), // "onSelectionChanged"
QT_MOC_LITERAL(15, 217, 8), // "onMoveUp"
QT_MOC_LITERAL(16, 226, 10) // "onMoveDown"

    },
    "MenuTreeWidget\0itemSelected\0\0filename\0"
    "itemType\0addLauncherRequested\0"
    "addDirectoryRequested\0addSeparatorRequested\0"
    "removeRequested\0moveUpRequested\0"
    "moveDownRequested\0reloadRequired\0"
    "onSearchChanged\0text\0onSelectionChanged\0"
    "onMoveUp\0onMoveDown"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MenuTreeWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   74,    2, 0x06 /* Public */,
       5,    0,   79,    2, 0x06 /* Public */,
       6,    0,   80,    2, 0x06 /* Public */,
       7,    0,   81,    2, 0x06 /* Public */,
       8,    0,   82,    2, 0x06 /* Public */,
       9,    0,   83,    2, 0x06 /* Public */,
      10,    0,   84,    2, 0x06 /* Public */,
      11,    0,   85,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    1,   86,    2, 0x08 /* Private */,
      14,    0,   89,    2, 0x08 /* Private */,
      15,    0,   90,    2, 0x08 /* Private */,
      16,    0,   91,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MenuTreeWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MenuTreeWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->itemSelected((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->addLauncherRequested(); break;
        case 2: _t->addDirectoryRequested(); break;
        case 3: _t->addSeparatorRequested(); break;
        case 4: _t->removeRequested(); break;
        case 5: _t->moveUpRequested(); break;
        case 6: _t->moveDownRequested(); break;
        case 7: _t->reloadRequired(); break;
        case 8: _t->onSearchChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->onSelectionChanged(); break;
        case 10: _t->onMoveUp(); break;
        case 11: _t->onMoveDown(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MenuTreeWidget::*)(const QString & , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MenuTreeWidget::itemSelected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MenuTreeWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MenuTreeWidget::addLauncherRequested)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MenuTreeWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MenuTreeWidget::addDirectoryRequested)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MenuTreeWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MenuTreeWidget::addSeparatorRequested)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (MenuTreeWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MenuTreeWidget::removeRequested)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (MenuTreeWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MenuTreeWidget::moveUpRequested)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (MenuTreeWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MenuTreeWidget::moveDownRequested)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (MenuTreeWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MenuTreeWidget::reloadRequired)) {
                *result = 7;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MenuTreeWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MenuTreeWidget.data,
    qt_meta_data_MenuTreeWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MenuTreeWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MenuTreeWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MenuTreeWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MenuTreeWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void MenuTreeWidget::itemSelected(const QString & _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MenuTreeWidget::addLauncherRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void MenuTreeWidget::addDirectoryRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void MenuTreeWidget::addSeparatorRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void MenuTreeWidget::removeRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void MenuTreeWidget::moveUpRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void MenuTreeWidget::moveDownRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void MenuTreeWidget::reloadRequired()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
