QT       += core gui xml printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

RC_ICONS = HeroSystem.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += ISHSC=1

SOURCES += \
    ../../HSCCU/HSCCU/character.cpp \
    ../../HSCCU/HSCCU/characteristic.cpp \
    ../../HSCCU/HSCCU/complication.cpp \
    ../../HSCCU/HSCCU/fraction.cpp \
    ../../HSCCU/HSCCU/modifier.cpp \
    ../../HSCCU/HSCCU/option.cpp \
    ../../HSCCU/HSCCU/powers.cpp \
    ../../HSCCU/HSCCU/skilltalentorperk.cpp \
    hsccharacter.cpp \
    main.cpp \
    mainwindow.cpp \
    setupdialog.cpp \
    simpledialog.cpp

HEADERS += \
    ../../HSCCU/HSCCU/character.h \
    hsccharacter.h \
    mainwindow.h \
    setupdialog.h \
    shared.h \
    simpledialog.h

FORMS += \
    mainwindow.ui \
    setupdialog.ui \
    simpledialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../Installer/config/config.xml \
    ../Installer/packages/com.vendor.product/meta/license.txt \
    ../Installer/packages/com.vendor.product/meta/package.xml \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    white_knight.dump

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
