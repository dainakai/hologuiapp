QT += widgets core gui
CONFIG += c++17

# OSに応じた設定
macx {
    INCLUDEPATH += /opt/homebrew/include
    LIBS += -L/opt/homebrew/lib
}

linux {
    INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt5 /usr/include/x86_64-linux-gnu/qt5/QtWidgets /usr/include/x86_64-linux-gnu/qt5/QtGui /usr/include/x86_64-linux-gnu/qt5/ /usr/include/opencv4 /usr/include/opencv4/opencv2/ /usr/loca/include -fPIC
    LIBS += 
}

# OpenCV設定
LIBS += -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui

# FFTW設定
LIBS += -lfftw3

OBJECTS_DIR = .obj
MOC_DIR = .obj
UI_DIR = .obj
# SRC_DIR = src

# ソースとヘッダ
SOURCES += src/*
HEADERS += src/*
DESTDIR = build

# ターゲット
TARGET = MyApp
