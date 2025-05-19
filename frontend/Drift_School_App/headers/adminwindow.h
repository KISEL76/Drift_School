#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QMovie>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDate>
#include <QTime>
#include <QDateTime>

#include "addinstructorwindow.h"
#include "firewindow.h"
#include "mainwindow.h"

namespace Ui {
class AdminWindow;
}

class AdminWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AdminWindow(const QString& token, QWidget *parent = nullptr);
    ~AdminWindow();

private slots:
    void on_pushButton_exit_clicked();
    void on_pushButton_create_appointment_clicked();
    void on_pushButton_delete_appointment_clicked();
    void on_pushButton_add_instructor_clicked();
    void on_pushButton_fire_instructor_clicked();

private:
    Ui::AdminWindow *ui;
    QString token;

    void fillLabels();
    void setupDateTimeLimits();
    void loadMastersIntoComboBox();
    void showSchedule();
};

#endif // ADMINWINDOW_H
