QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    src/boardwidget.cpp \
    src/mainwindow.cpp \
    src/initialdialog.cpp \
    src/newgamedialog.cpp \
    src/loadgamedialog.cpp \
    src/leaderboarddialog.cpp \
    src/Advisor.cpp \
    src/Board.cpp \
    src/Cannon.cpp \
    src/ChessPiece.cpp \
    src/Elephant.cpp \
    src/Game.cpp \
    src/King.cpp \
    src/Knight.cpp \
    src/Pawn.cpp \
    src/Position.cpp \
    src/Rook.cpp \
    src/SaveManager.cpp \
    src/Timer.cpp

HEADERS += \
    include/boardwidget.h \
    include/mainwindow.h \
    include/initialdialog.h \
    include/newgamedialog.h \
    include/loadgamedialog.h \
    include/leaderboarddialog.h \
    include/Advisor.h \
    include/Board.h \
    include/Cannon.h \
    include/ChessPiece.h \
    include/Elephant.h \
    include/Game.h \
    include/King.h \
    include/Knight.h \
    include/Pawn.h \
    include/Position.h \
    include/Rook.h \
    include/SaveManager.h \
    include/Timer.h

INCLUDEPATH += include

# 明确指定 MSYS2 MinGW 编译器（避免 qmake 自动检测失败）
QMAKE_CC = D:/project/QtProject/ChineseChess/gcc-msys2.bat
QMAKE_CXX = D:/project/QtProject/ChineseChess/g++-msys2.bat
QMAKE_LINK = D:/project/QtProject/ChineseChess/g++-msys2.bat

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
