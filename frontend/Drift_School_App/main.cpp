#include "headers/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    int fontId = QFontDatabase::addApplicationFont(":/assets/fonts/Swanston-Bold.ttf");
    if (fontId == -1)
    {
        qDebug() << "Ошибка загрузки шрифта!";
    }
    else
    {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont appFont(fontFamily, 20);
        QApplication::setFont(appFont);
    }

    QFile file(":/style.qss");
    if (file.open(QFile::ReadOnly))
    {
        QTextStream stream(&file);
        QString style = stream.readAll();
        a.setStyleSheet(style);
        file.close();
    }
    else
    {
        qWarning("Не удалось загрузить файл стилей");
    }

    MainWindow w;
    w.show();
    return a.exec();
}
