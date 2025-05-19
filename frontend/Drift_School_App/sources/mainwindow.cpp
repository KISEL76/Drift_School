#include "headers/mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Pro100 Drift School");
    setWindowIcon(QIcon(":/assets/icon/icon.JPG"));

    fillLabels();

    ui->lineEdit_nickname->setMaxLength(20);
    ui->lineEdit_password->setMaxLength(30);

    connect(ui->lineEdit_password, &QLineEdit::returnPressed, this, &MainWindow::on_pushButton_log_in_clicked);
    connect(ui->lineEdit_nickname, &QLineEdit::returnPressed, this, &MainWindow::on_pushButton_log_in_clicked);
    connect(ui->pushButton_sign_up, &QPushButton::clicked, this, &MainWindow::openSignUpForm);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openSignUpForm()
{
    SignUpWindow dialog(this);
    dialog.setModal(true);
    dialog.exec();
}

void MainWindow::on_pushButton_log_in_clicked()
{
    QString nickname = ui->lineEdit_nickname->text();
    QString password = ui->lineEdit_password->text();
    if (nickname.isEmpty() || password.isEmpty()) {
        QMessageBox::information(this, "Внимание", "Пожалуйста, заполните все поля.");
        return;
    }

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QUrl url("http://localhost:8080/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject payload;
    payload["nickname"] = nickname;
    payload["password"] = password;

    QNetworkReply* reply = manager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "Ошибка подключения", "Не удалось подключиться к серверу. Проверьте подключение.");
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj.contains("error")) {
            QMessageBox::warning(this, "Ошибка", obj["error"].toString());
            reply->deleteLater();
            return;
        }

        int userId = obj["user_id"].toInt();
        QString role = obj["role"].toString();
        this->token = obj["token"].toString();

        if (role == "admin") {
            AdminWindow* adminWindow = new AdminWindow(token);
            adminWindow->show();
        }
        else {
            UserWindow* userWindow = new UserWindow(userId, this->token);
            userWindow->show();
        }

        reply->deleteLater();
        this->close();
    });
}

void MainWindow::fillLabels()
{
    ui->label_car->setPixmap(QPixmap(":/assets/labels/bmw_m4.png"));
    ui->label_car->setScaledContents(true);
    ui->label_car->setStyleSheet("background: transparent;");
    ui->label_tyres->setPixmap(QPixmap(":/assets/labels/tyres.png"));
    ui->label_tyres->setScaledContents(true);
    ui->label_tyres->setStyleSheet("background: transparent;");
    ui->label_tyreTrack->setPixmap(QPixmap(":/assets/labels/tyres_track.png"));
    ui->label_tyreTrack->setScaledContents(true);
    ui->label_tyreTrack->setStyleSheet("background: transparent;");
}



