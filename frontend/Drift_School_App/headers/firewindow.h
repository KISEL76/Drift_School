#ifndef FIREWINDOW_H
#define FIREWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QMovie>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Ui {
class FireWindow;
}

class FireWindow : public QDialog
{
    Q_OBJECT

public:
    explicit FireWindow(const QString& token, QWidget *parent = nullptr);
    ~FireWindow();

private slots:
    void on_pushButton_fire_clicked();

private:
    Ui::FireWindow *ui;
    QString token;

    void loadMastersIntoComboBox();
    void fillLabels();
};

#endif // FIREWINDOW_H
