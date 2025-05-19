#include "headers/firewindow.h"
#include "ui_firewindow.h"

FireWindow::FireWindow(const QString& token, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FireWindow),
    token(token)
{
    ui->setupUi(this);

    setWindowTitle("Удаление инструктора");
    setWindowIcon(QIcon(":/assets/icon/icon.JPG"));

    loadMastersIntoComboBox();

    fillLabels();
}

FireWindow::~FireWindow()
{
    delete ui;
}

void FireWindow::loadMastersIntoComboBox()
{
    ui->comboBox_instructors->clear();

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://localhost:8080/instructors"));
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "Ошибка", "Ошибка подключения к серверу.");
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray arr = doc.array();

        for (const QJsonValue& val : arr) {
            QJsonObject obj = val.toObject();
            QString fullName = obj["first_name"].toString() + " " + obj["last_name"].toString();
            int instructorId = obj["instructor_id"].toInt();

            ui->comboBox_instructors->addItem(fullName, instructorId);
        }

        reply->deleteLater();
    });
}

void FireWindow::on_pushButton_fire_clicked() {
    int instructorId = ui->comboBox_instructors->currentData().toInt();

    if (instructorId == 0) {
        QMessageBox::information(this, "Ошибка", "Пожалуйста, выберите инструктора для удаления.");
        return;
    }

    QJsonObject payload;
    payload["instructor_id"] = instructorId;

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://localhost:8080/instructors/fire"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = manager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "Ошибка сети", "Не удалось подключиться к серверу.");
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj.contains("error")) {
            QMessageBox::critical(this, "Ошибка", obj["error"].toString());
        } else {
            QMessageBox::information(this, "Успех", "Инструктор успешно удалён!");
            loadMastersIntoComboBox();
        }

        reply->deleteLater();
    });
}

void FireWindow::fillLabels()
{
    QMovie *movie = new QMovie(":/assets/labels/Cillian_Murphy.gif");
    ui->label_dissapoint->setMovie(movie);
    ui->label_dissapoint->setScaledContents(true);
    movie->start();
}

