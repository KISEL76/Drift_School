#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCoreApplication>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QPixmap>
#include <QTextStream>
#include <QFontDatabase>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QNetworkReply>

#include "signupwindow.h"
#include "adminwindow.h"
#include "userwindow.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_log_in_clicked();

private:
    Ui::MainWindow *ui;

    void openSignUpForm();
    void fillLabels();

    QString token;

};
#endif // MAINWINDOW_H
