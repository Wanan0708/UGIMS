QT       += core gui network sql concurrent printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Explicit target name for the generated executable
TARGET = UGIMS

# Include paths - src作为根包含路径
INCLUDEPATH += src

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/ui/myform.cpp \
    src/ui/mapmanagerdialog.cpp \
    src/ui/analysispanel.cpp \
    src/ui/pipelineeditdialog.cpp \
    src/widgets/workordermanagerdialog.cpp \
    src/widgets/workordereditdialog.cpp \
    src/widgets/assetmanagerdialog.cpp \
    src/widgets/assetstatisticsdialog.cpp \
    src/widgets/facilityeditdialog.cpp \
    src/widgets/logindialog.cpp \
    src/widgets/customcombobox.cpp \
    src/widgets/usermanagerdialog.cpp \
    src/widgets/basewindow.cpp \
    src/widgets/customtitlebar.cpp \
    src/widgets/mapmanagersettings.cpp \
    src/widgets/drawingtoolpanel.cpp \
    src/widgets/layercontrolpanel.cpp \
    src/widgets/collapsiblegroupbox.cpp \
    src/widgets/entitypropertiesdialog.cpp \
    src/widgets/healthassessmentdialog.cpp \
    src/widgets/healthdevicelistdialog.cpp \
    src/widgets/settingsdialog.cpp \
    src/tilemap/tilemapmanager.cpp \
    src/tilemap/tileworker.cpp \
    src/tilemap/manifeststore.cpp \
    src/tilemap/downloadscheduler.cpp \
    src/core/common/logger.cpp \
    src/core/common/config.cpp \
    src/core/utils/idgenerator.cpp \
    src/core/database/databasemanager.cpp \
    src/core/models/pipeline.cpp \
    src/core/models/workorder.cpp \
    src/core/models/facility.cpp \
    src/core/models/user.cpp \
    src/core/auth/sessionmanager.cpp \
    src/core/auth/permissionmanager.cpp \
    src/core/workorder/workorderstatustransition.cpp \
    src/core/commands/drawcommand.cpp \
    src/core/io/drawingdatamanager.cpp \
    src/core/io/drawingdatabasemanager.cpp \
    src/dao/pipelinedao.cpp \
    src/dao/workorderdao.cpp \
    src/dao/facilitydao.cpp \
    src/dao/userdao.cpp \
    src/map/layermanager.cpp \
    src/map/symbolmanager.cpp \
    src/map/pipelinerenderer.cpp \
    src/map/facilityrenderer.cpp \
    src/map/annotationrenderer.cpp \
    src/map/mapdrawingmanager.cpp \
    src/analysis/spatialanalyzer.cpp \
    src/analysis/burstanalyzer.cpp \
    src/analysis/connectivityanalyzer.cpp \
    src/analysis/healthassessmentanalyzer.cpp

HEADERS += \
    src/ui/myform.h \
    src/ui/mapmanagerdialog.h \
    src/ui/analysispanel.h \
    src/ui/pipelineeditdialog.h \
    src/widgets/workordermanagerdialog.h \
    src/widgets/workordereditdialog.h \
    src/widgets/assetmanagerdialog.h \
    src/widgets/assetstatisticsdialog.h \
    src/widgets/facilityeditdialog.h \
    src/widgets/logindialog.h \
    src/widgets/customcombobox.h \
    src/widgets/usermanagerdialog.h \
    src/widgets/basewindow.h \
    src/widgets/customtitlebar.h \
    src/widgets/mapmanagersettings.h \
    src/widgets/drawingtoolpanel.h \
    src/widgets/layercontrolpanel.h \
    src/widgets/collapsiblegroupbox.h \
    src/widgets/entitypropertiesdialog.h \
    src/widgets/healthassessmentdialog.h \
    src/widgets/healthdevicelistdialog.h \
    src/widgets/settingsdialog.h \
    src/tilemap/tilemapmanager.h \
    src/tilemap/tileworker.h \
    src/tilemap/manifeststore.h \
    src/tilemap/downloadscheduler.h \
    src/core/common/logger.h \
    src/core/common/config.h \
    src/core/utils/idgenerator.h \
    src/core/database/databasemanager.h \
    src/core/models/pipeline.h \
    src/core/models/workorder.h \
    src/core/models/facility.h \
    src/core/models/user.h \
    src/core/auth/sessionmanager.h \
    src/core/auth/permissionmanager.h \
    src/core/workorder/workorderstatustransition.h \
    src/core/commands/drawcommand.h \
    src/core/io/drawingdatamanager.h \
    src/core/io/drawingdatabasemanager.h \
    src/dao/basedao.h \
    src/dao/pipelinedao.h \
    src/dao/workorderdao.h \
    src/dao/facilitydao.h \
    src/dao/userdao.h \
    src/map/layermanager.h \
    src/map/symbolmanager.h \
    src/map/pipelinerenderer.h \
    src/map/facilityrenderer.h \
    src/map/annotationrenderer.h \
    src/map/mapdrawingmanager.h \
    src/analysis/spatialanalyzer.h \
    src/analysis/burstanalyzer.h \
    src/analysis/connectivityanalyzer.h \
    src/analysis/healthassessmentanalyzer.h

FORMS += \
    src/ui/myform.ui

RESOURCES += \
    resources/image.qrc \
    resources/style.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
