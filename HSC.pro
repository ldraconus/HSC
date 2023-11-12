QT       += core gui xml printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20 no_batch

RC_ICONS = HeroSystem.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += ISHSC=1

SOURCES += \
    ../../HSCCU/HSCCU/Equipment.cpp \
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
    ../../HSCCU/HSCCU/AccidentalChange.h \
    ../../HSCCU/HSCCU/AdjustmentPowers.h \
    ../../HSCCU/HSCCU/AgilitySkills.h \
    ../../HSCCU/HSCCU/AttackPowers.h \
    ../../HSCCU/HSCCU/AutomatonPowers.h \
    ../../HSCCU/HSCCU/BackgroundSkills.h \
    ../../HSCCU/HSCCU/BodyAffectingPowers.h \
    ../../HSCCU/HSCCU/CombatSkills.h \
    ../../HSCCU/HSCCU/Debug.h \
    ../../HSCCU/HSCCU/DefensePowers.h \
    ../../HSCCU/HSCCU/Dependence.h \
    ../../HSCCU/HSCCU/Dependent.h \
    ../../HSCCU/HSCCU/DistinctiveFeature.h \
    ../../HSCCU/HSCCU/Enraged.h \
    ../../HSCCU/HSCCU/Equipment.h \
    ../../HSCCU/HSCCU/FrameworkPowers.h \
    ../../HSCCU/HSCCU/Hunted.h \
    ../../HSCCU/HSCCU/IntellectSkills.h \
    ../../HSCCU/HSCCU/InteractionSkills.h \
    ../../HSCCU/HSCCU/MentalPowers.h \
    ../../HSCCU/HSCCU/MiscSkill.h \
    ../../HSCCU/HSCCU/Money.h \
    ../../HSCCU/HSCCU/MovementPowers.h \
    ../../HSCCU/HSCCU/NegativeReputation.h \
    ../../HSCCU/HSCCU/Perk.h \
    ../../HSCCU/HSCCU/PhysicalComplication.h \
    ../../HSCCU/HSCCU/PsychologicalComplication.h \
    ../../HSCCU/HSCCU/Rivalry.h \
    ../../HSCCU/HSCCU/SenseAffectingPowers.h \
    ../../HSCCU/HSCCU/SensoryPowers.h \
    ../../HSCCU/HSCCU/SkillEnhancers.h \
    ../../HSCCU/HSCCU/SocialComplication.h \
    ../../HSCCU/HSCCU/SpecialPowers.h \
    ../../HSCCU/HSCCU/StandardPowers.h \
    ../../HSCCU/HSCCU/Susceptibility.h \
    ../../HSCCU/HSCCU/Talent.h \
    ../../HSCCU/HSCCU/Unluck.h \
    ../../HSCCU/HSCCU/Vulnerability.h \
    ../../HSCCU/HSCCU/character.h \
    ../../HSCCU/HSCCU/characteristic.h \
    ../../HSCCU/HSCCU/complication.h \
    ../../HSCCU/HSCCU/fraction.h \
    ../../HSCCU/HSCCU/modifier.h \
    ../../HSCCU/HSCCU/option.h \
    ../../HSCCU/HSCCU/powers.h \
    ../../HSCCU/HSCCU/shared.h \
    ../../HSCCU/HSCCU/skilltalentorperk.h \
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
    Installer/config/Style.qss \
    Installer/config/background.png \
    Installer/config/config.xml \
    Installer/packages/com.vendor.product/meta/license.txt \
    Installer/packages/com.vendor.product/meta/package.xml \
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
