/****************************************************************************
** Meta object code from reading C++ file 'applicationeditorwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.10)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "applicationeditorwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'applicationeditorwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.10. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ApplicationEditorWidget_t {
    QByteArrayData data[13];
    char stringdata0[159];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ApplicationEditorWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ApplicationEditorWidget_t qt_meta_stringdata_ApplicationEditorWidget = {
    {
QT_MOC_LITERAL(0, 0, 23), // "ApplicationEditorWidget"
QT_MOC_LITERAL(1, 24, 12), // "entryChanged"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 8), // "filePath"
QT_MOC_LITERAL(4, 47, 13), // "saveRequested"
QT_MOC_LITERAL(5, 61, 14), // "closeRequested"
QT_MOC_LITERAL(6, 76, 14), // "cloneRequested"
QT_MOC_LITERAL(7, 91, 12), // "DesktopEntry"
QT_MOC_LITERAL(8, 104, 5), // "entry"
QT_MOC_LITERAL(9, 110, 19), // "onIconButtonClicked"
QT_MOC_LITERAL(10, 130, 6), // "onSave"
QT_MOC_LITERAL(11, 137, 7), // "onClone"
QT_MOC_LITERAL(12, 145, 13) // "onFieldEdited"

    },
    "ApplicationEditorWidget\0entryChanged\0"
    "\0filePath\0saveRequested\0closeRequested\0"
    "cloneRequested\0DesktopEntry\0entry\0"
    "onIconButtonClicked\0onSave\0onClone\0"
    "onFieldEdited"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ApplicationEditorWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    0,   57,    2, 0x06 /* Public */,
       5,    0,   58,    2, 0x06 /* Public */,
       6,    1,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   62,    2, 0x08 /* Private */,
      10,    0,   63,    2, 0x08 /* Private */,
      11,    0,   64,    2, 0x08 /* Private */,
      12,    0,   65,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ApplicationEditorWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ApplicationEditorWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->entryChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->saveRequested(); break;
        case 2: _t->closeRequested(); break;
        case 3: _t->cloneRequested((*reinterpret_cast< const DesktopEntry(*)>(_a[1]))); break;
        case 4: _t->onIconButtonClicked(); break;
        case 5: _t->onSave(); break;
        case 6: _t->onClone(); break;
        case 7: _t->onFieldEdited(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ApplicationEditorWidget::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplicationEditorWidget::entryChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ApplicationEditorWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplicationEditorWidget::saveRequested)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ApplicationEditorWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplicationEditorWidget::closeRequested)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ApplicationEditorWidget::*)(const DesktopEntry & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ApplicationEditorWidget::cloneRequested)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ApplicationEditorWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QScrollArea::staticMetaObject>(),
    qt_meta_stringdata_ApplicationEditorWidget.data,
    qt_meta_data_ApplicationEditorWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ApplicationEditorWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ApplicationEditorWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ApplicationEditorWidget.stringdata0))
        return static_cast<void*>(this);
    return QScrollArea::qt_metacast(_clname);
}

int ApplicationEditorWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QScrollArea::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void ApplicationEditorWidget::entryChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ApplicationEditorWidget::saveRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ApplicationEditorWidget::closeRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ApplicationEditorWidget::cloneRequested(const DesktopEntry & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
