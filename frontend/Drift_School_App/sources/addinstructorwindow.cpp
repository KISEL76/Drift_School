#include "headers/addinstructorwindow.h"
#include "ui_addinstructorwindow.h"

AddInstructorWindow::AddInstructorWindow(const QString& token, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddInstructorWindow),
    token(token)
{
    ui->setupUi(this);

    setWindowTitle("Добавление нового инструктора");
    setWindowIcon(QIcon(":/assets/icon/icon.JPG"));

    fillLabels();

    addRegex();
}

AddInstructorWindow::~AddInstructorWindow()
{
    delete ui;
}

void AddInstructorWindow::on_pushButton_add_clicked()
{
    if (ui->lineEdit_firstName->text().isEmpty() ||
        ui->lineEdit_lastName->text().isEmpty() ||
        ui->lineEdit_specialization->text().isEmpty() ||
        ui->lineEdit_phoneNumber->text().isEmpty() ||
        ui->lineEdit_email->text().isEmpty()) {
        QMessageBox::information(this, "Дорогой админ!", "Все поля должны быть заполнены!");
        return;
    }

    QString firstName = ui->lineEdit_firstName->text();
    QString lastName = ui->lineEdit_lastName->text();
    QString specialization = ui->lineEdit_specialization->text();
    QString phoneNumber = ui->lineEdit_phoneNumber->text();
    QString email = ui->lineEdit_email->text();

    QJsonObject payload;
    payload["first_name"] = firstName;
    payload["last_name"] = lastName;
    payload["specialization"] = specialization;
    payload["phone_number"] = phoneNumber;
    payload["email"] = email;

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://localhost:8080/instructors"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = manager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj.contains("error")) {
            QMessageBox::critical(this, "Ошибка", obj["error"].toString());
        } else {
            QMessageBox::information(this, "Успех", "Инструктор успешно добавлен!");
            accept();
        }
    });
}

void AddInstructorWindow::fillLabels()
{
    ui->label_MaxVerstappen->setPixmap(QPixmap(":/assets/labels/Max_Verstappen.png"));
    ui->label_MaxVerstappen->setScaledContents(true);
    ui->label_MaxVerstappen->setStyleSheet("background: transparent;");
}

void AddInstructorWindow::addRegex()
{
    QRegularExpression nameRegex("^[А-Яа-яЁё]{1,20}$");
    QRegularExpressionValidator *nameValidator = new QRegularExpressionValidator(nameRegex, this);
    ui->lineEdit_firstName->setValidator(nameValidator);
    ui->lineEdit_lastName->setValidator(nameValidator);

    QRegularExpression emailRegex(R"((^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$))");
    QRegularExpressionValidator *emailValidator = new QRegularExpressionValidator(emailRegex, this);
    ui->lineEdit_email->setValidator(emailValidator);
    ui->lineEdit_email->setPlaceholderText("example@domain.com");

    ui->lineEdit_specialization->setMaxLength(350);

    ui->lineEdit_phoneNumber->setInputMask("+7 (000) 000-00-00");
}
