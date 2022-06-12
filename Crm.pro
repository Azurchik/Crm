QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    windows/AddRecordDialog.cpp \
    CrmTableModel.cpp \
    DatabaseController.cpp \
    main.cpp \
    MainWindow.cpp \
    windows/ClientDialog.cpp \
    windows/FindClientDialog.cpp \
    windows/RecordDialog.cpp \
    windows/ServicesDialog.cpp \
    windows/WorkersDialog.cpp

HEADERS += \
    Consts.h \
    windows/AddRecordDialog.h \
    CrmTableModel.h \
    DatabaseController.h \
    MainWindow.h \
    windows/ClientDialog.h \
    windows/FindClientDialog.h \
    windows/RecordDialog.h \
    windows/ServicesDialog.h \
    windows/WorkersDialog.h

FORMS += \
    windows/AddRecordDialog.ui \
    MainWindow.ui \
    windows/ClientDialog.ui \
    windows/FindClientDialog.ui \
    windows/RecordDialog.ui \
    windows/ServicesDialog.ui \
    windows/WorkersDialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
