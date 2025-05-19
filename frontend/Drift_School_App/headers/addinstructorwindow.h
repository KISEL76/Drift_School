#ifndef ADDINSTRUCTORWINDOW_H
#define ADDINSTRUCTORWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

namespace Ui {
class AddInstructorWindow;
}

class AddInstructorWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddInstructorWindow(const QString& token, QWidget *parent = nullptr);

    ~AddInstructorWindow();

private:
    Ui::AddInstructorWindow *ui;

    void fillLabels();
    void addRegex();

    QString token;

private slots:
    void on_pushButton_add_clicked();
};

#endif // ADDINSTRUCTORWINDOW_H
