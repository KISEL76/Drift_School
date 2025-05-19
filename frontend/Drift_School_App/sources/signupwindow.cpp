#include "headers/signupwindow.h"
#include "ui_signupwindow.h"

SignUpWindow::SignUpWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::SignUpWindow)
{
    ui->setupUi(this);

    setWindowTitle("Окно регистрации");
    setWindowIcon(QIcon(":/assets/icon/icon.JPG"));

    fillLabels();

    addRegex();
}

SignUpWindow::~SignUpWindow()
{
    delete ui;
}

void SignUpWindow::on_pushButton_sign_up_clicked()
{
    if (ui->lineEdit_birth_date->displayText().isEmpty() ||
        ui->lineEdit_email->displayText().isEmpty() ||
        ui->lineEdit_first_name->displayText().isEmpty() ||
        ui->lineEdit_last_name->displayText().isEmpty() ||
        ui->lineEdit_password->displayText().isEmpty() ||
        ui->lineEdit_phone_number->displayText().isEmpty() ||
        ui->lineEdit_nickname->displayText().isEmpty()) {
        QMessageBox::information(this, "Внимание", "Все поля должны быть заполнены.");
        return;
    }

    QString nickname = ui->lineEdit_nickname->text();
    QString phoneNumber = ui->lineEdit_phone_number->text();
    QString email = ui->lineEdit_email->text();
    QString firstName = ui->lineEdit_first_name->text();
    QString lastName = ui->lineEdit_last_name->text();
    QString birthDate = ui->lineEdit_birth_date->text();
    QString password = ui->lineEdit_password->text();

    QDate birthDateToForm = QDate::fromString(birthDate, "dd.MM.yyyy");
    if (!birthDateToForm.isValid()) {
        QMessageBox::information(this, "Внимание", "Введен некорректный формат даты рождения.");
        return;
    }

    QString formattedDate = birthDateToForm.toString("yyyy-MM-dd");

    QJsonObject payload;
    payload["nickname"] = nickname;
    payload["phone_number"] = phoneNumber;
    payload["email"] = email;
    payload["first_name"] = firstName;
    payload["last_name"] = lastName;
    payload["birth_date"] = formattedDate;
    payload["password"] = password;

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://localhost:8080/signup"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = manager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "Ошибка сети", "Не удалось подключиться к серверу. Проверьте подключение.");
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj.contains("error")) {
            QMessageBox::critical(this, "Ошибка", obj["error"].toString());
        }
        else {
            QMessageBox::information(this, "Это успех", "Вы успешно зарегистрировались!");
            this->accept();
        }
    });
}

void SignUpWindow::addRegex()
{
    QRegularExpression regex_nickname("^[A-Za-z0-9]{1,15}$");
    QRegularExpressionValidator* validator_nickname = new QRegularExpressionValidator(regex_nickname, this);
    ui->lineEdit_nickname->setValidator(validator_nickname);

    QRegularExpression regex_name("^[A-Za-zА-Яа-яЁё]{1,20}$");
    QRegularExpressionValidator* validator_name = new QRegularExpressionValidator(regex_name, this);
    ui->lineEdit_first_name->setValidator(validator_name);
    ui->lineEdit_last_name->setValidator(validator_name);

    QRegularExpression regex_email(R"((^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$))");
    QRegularExpressionValidator *validator_email = new QRegularExpressionValidator(regex_email, this);
    ui->lineEdit_email->setValidator(validator_email);
    ui->lineEdit_email->setPlaceholderText("example@domain.com");
    ui->lineEdit_email->setMaxLength(30);

    QRegularExpression regex_password("^[A-Za-z0-9_!@#$%^&*()\\-+=<>?,./:;{}\\[\\]~]{1,30}$");
    QRegularExpressionValidator* validator_password = new QRegularExpressionValidator(regex_password, this);
    ui->lineEdit_password->setValidator(validator_password);
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);

    ui->lineEdit_phone_number->setInputMask("+7 (000) 000-00-00");
    ui->lineEdit_birth_date->setInputMask("00.00.0000");
}

void SignUpWindow::fillLabels()
{
    QMovie *movie = new QMovie(":/assets/labels/Toretto.gif");
    ui->label_Toretto->setMovie(movie);
    ui->label_Toretto->setScaledContents(true);
    movie->start();
}
