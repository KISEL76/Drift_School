#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QDialog>
#include <QIcon>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QPixmap>
#include <QDateTime>
#include <QMovie>

#include "mainwindow.h"

namespace Ui {
class UserWindow;
}

class UserWindow : public QDialog
{
    Q_OBJECT

public:
    explicit UserWindow(int userId, const QString& token, QWidget *parent = nullptr);
    ~UserWindow();

private slots:
    void on_pushButton_review_clicked();
    void on_pushButton_exit_clicked();
    void onScheduleItemChanged(QTableWidgetItem *item);

private:
    Ui::UserWindow *ui;

    int userId;

    QString token;

    QNetworkAccessManager* networkManager;

    void showServices();
    void showRatings();
    void showSchedule();
    void showMasters();
    void fillLabels();

    void createOrder(int userId, int masterId, int serviceId, const QString &orderDate, double totalPrice);
    void deleteOrder(int userId, int masterId, const QString &orderDate);
};

#endif // USERWINDOW_H
