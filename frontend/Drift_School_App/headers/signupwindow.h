#ifndef SIGNUPWINDOW_H
#define SIGNUPWINDOW_H

#include <QDialog>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QDebug>
#include <QDate>
#include <QMessageBox>
#include <QMovie>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

namespace Ui {
    class SignUpWindow;
}

class SignUpWindow : public QDialog
{
    Q_OBJECT
public:
    explicit SignUpWindow(QWidget *parent = nullptr);
    ~SignUpWindow();

private:
    Ui::SignUpWindow *ui;

    void addRegex();
    void fillLabels();

private slots:
    void on_pushButton_sign_up_clicked();
};

#endif // SIGNUPWINDOW_H
