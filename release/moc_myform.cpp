/****************************************************************************
** Meta object code from reading C++ file 'myform.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/ui/myform.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'myform.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN6MyFormE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN6MyFormE = QtMocHelpers::stringData(
    "MyForm",
    "handleNewButtonClicked",
    "",
    "handleOpenButtonClicked",
    "handleSaveButtonClicked",
    "handleSaveAsButtonClicked",
    "handleUndoButtonClicked",
    "handleRedoButtonClicked",
    "handleLoadMapButtonClicked",
    "handleZoomInButtonClicked",
    "handleZoomOutButtonClicked",
    "handlePanButtonClicked",
    "handleLoadTileMapButtonClicked",
    "handleZoomInTileMapButtonClicked",
    "handleZoomOutTileMapButtonClicked",
    "onTileDownloadProgress",
    "current",
    "total",
    "onRegionDownloadProgress",
    "zoom",
    "onOverlayPanToggled",
    "checked",
    "loadPipelineData",
    "onViewTransformChanged",
    "onLoadDataButtonClicked",
    "onDownloadMapButtonClicked",
    "onBurstAnalysisButtonClicked",
    "onConnectivityAnalysisButtonClicked",
    "onWorkOrderButtonClicked",
    "onAssetManagementButtonClicked",
    "onSettingsButtonClicked",
    "onHelpButtonClicked",
    "onDeviceTreeItemClicked",
    "QModelIndex",
    "index",
    "onDeviceTreeItemDoubleClicked",
    "onDeviceSearchTextChanged",
    "text",
    "onAboutButtonClicked",
    "onToggleDrawingTool",
    "onStartDrawingPipeline",
    "pipelineType",
    "onStartDrawingFacility",
    "facilityType",
    "onPipelineDrawingFinished",
    "wkt",
    "QList<QPointF>",
    "points",
    "onFacilityDrawingFinished",
    "point",
    "onEntityClicked",
    "QGraphicsItem*",
    "item",
    "onEntityDoubleClicked",
    "onShowContextMenu",
    "pos",
    "onDeleteSelectedEntity",
    "onEditSelectedEntity",
    "onViewEntityProperties",
    "clearSelection"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN6MyFormE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      42,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  266,    2, 0x08,    1 /* Private */,
       3,    0,  267,    2, 0x08,    2 /* Private */,
       4,    0,  268,    2, 0x08,    3 /* Private */,
       5,    0,  269,    2, 0x08,    4 /* Private */,
       6,    0,  270,    2, 0x08,    5 /* Private */,
       7,    0,  271,    2, 0x08,    6 /* Private */,
       8,    0,  272,    2, 0x08,    7 /* Private */,
       9,    0,  273,    2, 0x08,    8 /* Private */,
      10,    0,  274,    2, 0x08,    9 /* Private */,
      11,    0,  275,    2, 0x08,   10 /* Private */,
      12,    0,  276,    2, 0x08,   11 /* Private */,
      13,    0,  277,    2, 0x08,   12 /* Private */,
      14,    0,  278,    2, 0x08,   13 /* Private */,
      15,    2,  279,    2, 0x08,   14 /* Private */,
      18,    3,  284,    2, 0x08,   17 /* Private */,
      20,    1,  291,    2, 0x08,   21 /* Private */,
      22,    0,  294,    2, 0x08,   23 /* Private */,
      23,    0,  295,    2, 0x08,   24 /* Private */,
      24,    0,  296,    2, 0x08,   25 /* Private */,
      25,    0,  297,    2, 0x08,   26 /* Private */,
      26,    0,  298,    2, 0x08,   27 /* Private */,
      27,    0,  299,    2, 0x08,   28 /* Private */,
      28,    0,  300,    2, 0x08,   29 /* Private */,
      29,    0,  301,    2, 0x08,   30 /* Private */,
      30,    0,  302,    2, 0x08,   31 /* Private */,
      31,    0,  303,    2, 0x08,   32 /* Private */,
      32,    1,  304,    2, 0x08,   33 /* Private */,
      35,    1,  307,    2, 0x08,   35 /* Private */,
      36,    1,  310,    2, 0x08,   37 /* Private */,
      38,    0,  313,    2, 0x08,   39 /* Private */,
      39,    1,  314,    2, 0x08,   40 /* Private */,
      40,    1,  317,    2, 0x08,   42 /* Private */,
      42,    1,  320,    2, 0x08,   44 /* Private */,
      44,    3,  323,    2, 0x08,   46 /* Private */,
      48,    3,  330,    2, 0x08,   50 /* Private */,
      50,    1,  337,    2, 0x08,   54 /* Private */,
      53,    1,  340,    2, 0x08,   56 /* Private */,
      54,    1,  343,    2, 0x08,   58 /* Private */,
      56,    0,  346,    2, 0x08,   60 /* Private */,
      57,    0,  347,    2, 0x08,   61 /* Private */,
      58,    0,  348,    2, 0x08,   62 /* Private */,
      59,    0,  349,    2, 0x08,   63 /* Private */,

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
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   16,   17,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   16,   17,   19,
    QMetaType::Void, QMetaType::Bool,   21,
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
    QMetaType::Void, 0x80000000 | 33,   34,
    QMetaType::Void, 0x80000000 | 33,   34,
    QMetaType::Void, QMetaType::QString,   37,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   21,
    QMetaType::Void, QMetaType::QString,   41,
    QMetaType::Void, QMetaType::QString,   43,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, 0x80000000 | 46,   41,   45,   47,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QPointF,   43,   45,   49,
    QMetaType::Void, 0x80000000 | 51,   52,
    QMetaType::Void, 0x80000000 | 51,   52,
    QMetaType::Void, QMetaType::QPoint,   55,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MyForm::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN6MyFormE.offsetsAndSizes,
    qt_meta_data_ZN6MyFormE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN6MyFormE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MyForm, std::true_type>,
        // method 'handleNewButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleOpenButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSaveButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSaveAsButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleUndoButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleRedoButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleLoadMapButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleZoomInButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleZoomOutButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handlePanButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleLoadTileMapButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleZoomInTileMapButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleZoomOutTileMapButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTileDownloadProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onRegionDownloadProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onOverlayPanToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'loadPipelineData'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onViewTransformChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLoadDataButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDownloadMapButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBurstAnalysisButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onConnectivityAnalysisButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onWorkOrderButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAssetManagementButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSettingsButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onHelpButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDeviceTreeItemClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QModelIndex &, std::false_type>,
        // method 'onDeviceTreeItemDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QModelIndex &, std::false_type>,
        // method 'onDeviceSearchTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onAboutButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onToggleDrawingTool'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onStartDrawingPipeline'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onStartDrawingFacility'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onPipelineDrawingFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<QPointF> &, std::false_type>,
        // method 'onFacilityDrawingFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPointF &, std::false_type>,
        // method 'onEntityClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QGraphicsItem *, std::false_type>,
        // method 'onEntityDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QGraphicsItem *, std::false_type>,
        // method 'onShowContextMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>,
        // method 'onDeleteSelectedEntity'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEditSelectedEntity'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onViewEntityProperties'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'clearSelection'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MyForm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MyForm *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->handleNewButtonClicked(); break;
        case 1: _t->handleOpenButtonClicked(); break;
        case 2: _t->handleSaveButtonClicked(); break;
        case 3: _t->handleSaveAsButtonClicked(); break;
        case 4: _t->handleUndoButtonClicked(); break;
        case 5: _t->handleRedoButtonClicked(); break;
        case 6: _t->handleLoadMapButtonClicked(); break;
        case 7: _t->handleZoomInButtonClicked(); break;
        case 8: _t->handleZoomOutButtonClicked(); break;
        case 9: _t->handlePanButtonClicked(); break;
        case 10: _t->handleLoadTileMapButtonClicked(); break;
        case 11: _t->handleZoomInTileMapButtonClicked(); break;
        case 12: _t->handleZoomOutTileMapButtonClicked(); break;
        case 13: _t->onTileDownloadProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 14: _t->onRegionDownloadProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 15: _t->onOverlayPanToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 16: _t->loadPipelineData(); break;
        case 17: _t->onViewTransformChanged(); break;
        case 18: _t->onLoadDataButtonClicked(); break;
        case 19: _t->onDownloadMapButtonClicked(); break;
        case 20: _t->onBurstAnalysisButtonClicked(); break;
        case 21: _t->onConnectivityAnalysisButtonClicked(); break;
        case 22: _t->onWorkOrderButtonClicked(); break;
        case 23: _t->onAssetManagementButtonClicked(); break;
        case 24: _t->onSettingsButtonClicked(); break;
        case 25: _t->onHelpButtonClicked(); break;
        case 26: _t->onDeviceTreeItemClicked((*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 27: _t->onDeviceTreeItemDoubleClicked((*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 28: _t->onDeviceSearchTextChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 29: _t->onAboutButtonClicked(); break;
        case 30: _t->onToggleDrawingTool((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 31: _t->onStartDrawingPipeline((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 32: _t->onStartDrawingFacility((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 33: _t->onPipelineDrawingFinished((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QList<QPointF>>>(_a[3]))); break;
        case 34: _t->onFacilityDrawingFinished((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[3]))); break;
        case 35: _t->onEntityClicked((*reinterpret_cast< std::add_pointer_t<QGraphicsItem*>>(_a[1]))); break;
        case 36: _t->onEntityDoubleClicked((*reinterpret_cast< std::add_pointer_t<QGraphicsItem*>>(_a[1]))); break;
        case 37: _t->onShowContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 38: _t->onDeleteSelectedEntity(); break;
        case 39: _t->onEditSelectedEntity(); break;
        case 40: _t->onViewEntityProperties(); break;
        case 41: _t->clearSelection(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 33:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<QPointF> >(); break;
            }
            break;
        case 35:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QGraphicsItem* >(); break;
            }
            break;
        case 36:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QGraphicsItem* >(); break;
            }
            break;
        }
    }
}

const QMetaObject *MyForm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyForm::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN6MyFormE.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MyForm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 42)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 42;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 42)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 42;
    }
    return _id;
}
QT_WARNING_POP
