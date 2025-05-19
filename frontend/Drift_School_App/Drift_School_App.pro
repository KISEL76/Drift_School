QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

SOURCES += \
    main.cpp \
    sources/addinstructorwindow.cpp \
    sources/adminwindow.cpp \
    sources/firewindow.cpp \
    sources/signupwindow.cpp \
    sources/mainwindow.cpp \
    sources/userwindow.cpp

HEADERS += \
    headers/addinstructorwindow.h \
    headers/adminwindow.h \
    headers/firewindow.h \
    headers/signupwindow.h \
    headers/mainwindow.h \
    headers/userwindow.h

FORMS += \
    forms/addinstructorwindow.ui \
    forms/adminwindow.ui \
    forms/firewindow.ui \
    forms/mainwindow.ui \
    forms/signupwindow.ui \
    forms/userwindow.ui

RESOURCES += \
    resourses.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
