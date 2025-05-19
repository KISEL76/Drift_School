#include "headers/adminwindow.h"
#include "ui_adminwindow.h"

AdminWindow::AdminWindow(const QString& token, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdminWindow),
    token(token)
{
    ui->setupUi(this);

    setWindowTitle("Pro100 Drift School");
    setWindowIcon(QIcon(":/assets/icon/icon.JPG"));

    fillLabels();

    showSchedule();
    loadMastersIntoComboBox();
    setupDateTimeLimits();
}

AdminWindow::~AdminWindow() {
    delete ui;
}

void AdminWindow::on_pushButton_exit_clicked() {
    MainWindow *mainWindow = new MainWindow();
    mainWindow->show();
    this->close();
}

void AdminWindow::showSchedule() {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://localhost:8080/schedule"));
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray arr = doc.array();

        ui->tableWidget_schedule->setRowCount(arr.size());
        ui->tableWidget_schedule->setColumnCount(5);
        ui->tableWidget_schedule->setHorizontalHeaderLabels(
            {"Дата", "Инструктор", "Время начала", "Время окончания", "Забронировано"});

        int row = 0;
        for (const QJsonValue &val : arr) {
            QJsonObject obj = val.toObject();

            QDate date = QDate::fromString(obj["available_date"].toString(), "yyyy-MM-dd");
            QString formattedDate = date.isValid() ? date.toString("dd.MM.yyyy") : "Неверный формат";
            ui->tableWidget_schedule->setItem(row, 0, new QTableWidgetItem(formattedDate));

            QString fullName = obj["first_name"].toString() + " " + obj["last_name"].toString();
            ui->tableWidget_schedule->setItem(row, 1, new QTableWidgetItem(fullName));

            QTime start = QTime::fromString(obj["start_time"].toString(), "HH:mm:ss");
            QString formattedStart = start.isValid() ? start.toString("HH:mm") : "Неверно";
            ui->tableWidget_schedule->setItem(row, 2, new QTableWidgetItem(formattedStart));

            QTime end = QTime::fromString(obj["end_time"].toString(), "HH:mm:ss");
            QString formattedEnd = end.isValid() ? end.toString("HH:mm") : "Неверно";
            ui->tableWidget_schedule->setItem(row, 3, new QTableWidgetItem(formattedEnd));

            bool isBooked = obj["is_booked"].toBool();
            QTableWidgetItem* checkItem = new QTableWidgetItem();
            checkItem->setCheckState(isBooked ? Qt::Checked : Qt::Unchecked);
            checkItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            ui->tableWidget_schedule->setItem(row, 4, checkItem);

            row++;
        }

        ui->tableWidget_schedule->resizeColumnsToContents();
        ui->tableWidget_schedule->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableWidget_schedule->setSelectionBehavior(QAbstractItemView::SelectRows);
    });
}

void AdminWindow::loadMastersIntoComboBox() {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://localhost:8080/instructors"));
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray arr = doc.array();

        ui->comboBox_instructors->clear();
        for (const QJsonValue &val : arr) {
            QJsonObject obj = val.toObject();
            QString name = obj["first_name"].toString() + " " + obj["last_name"].toString();
            ui->comboBox_instructors->addItem(name);
        }
    });
}

void AdminWindow::on_pushButton_create_appointment_clicked() {
    QDate selectedDate = ui->dateEdit->date();
    QTime selectedStartTime = ui->timeEdit_start->time();
    QDateTime selectedDateTime(selectedDate, selectedStartTime);
    QDateTime currentDateTime = QDateTime::currentDateTime();

    if (selectedDateTime < currentDateTime) {
        QMessageBox::warning(this, "Ошибка", "Нельзя создать запись в прошлом.");
        return;
    }

    QString instructorName = ui->comboBox_instructors->currentText();
    QString date = selectedDate.toString("yyyy-MM-dd");
    QString startTime = selectedStartTime.toString("HH:mm");
    QString endTime = ui->timeEdit_end->time().toString("HH:mm");

    QStringList nameParts = instructorName.split(" ");
    if (nameParts.size() < 2) {
        QMessageBox::critical(this, "Ошибка", "Некорректное имя инструктора.");
        return;
    }

    QString firstName = nameParts[0];
    QString lastName = nameParts[1];

    QJsonObject payload;
    payload["first_name"] = firstName;
    payload["last_name"] = lastName;
    payload["available_date"] = date;
    payload["start_time"] = startTime;
    payload["end_time"] = endTime;

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://localhost:8080/schedule"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = manager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "Ошибка сети", reply->errorString());
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj.contains("error")) {
            QMessageBox::critical(this, "Ошибка", obj["error"].toString());
        } else {
            QMessageBox::information(this, "Успех", "Слот успешно создан!");
            showSchedule();
        }

        reply->deleteLater();
    });
}

void AdminWindow::on_pushButton_delete_appointment_clicked() {
    int currentRow = ui->tableWidget_schedule->currentRow();
    if (currentRow < 0) {
        QMessageBox::information(this, "Дорогой админ!", "Сначала нужно выбрать запись для удаления!");
        return;
    }

    QString instructorName = ui->tableWidget_schedule->item(currentRow, 1)->text();
    QString date = ui->tableWidget_schedule->item(currentRow, 0)->text();
    QString timeStart = ui->tableWidget_schedule->item(currentRow, 2)->text();

    QStringList nameParts = instructorName.split(" ");
    if (nameParts.size() != 2) {
        QMessageBox::critical(this, "Ошибка", "Некорректное имя инструктора.");
        return;
    }

    QString firstName = nameParts[0];
    QString lastName = nameParts[1];
    QDate sqlDate = QDate::fromString(date, "dd.MM.yyyy");
    QString formattedDate = sqlDate.toString("yyyy-MM-dd");

    QJsonObject payload;
    payload["first_name"] = firstName;
    payload["last_name"] = lastName;
    payload["available_date"] = formattedDate;
    payload["start_time"] = timeStart;

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://localhost:8080/schedule/delete"));
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

        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpStatus >= 400) {
            QString errMsg = obj.contains("error") ? obj["error"].toString() : "Неизвестная ошибка удаления.";
            QMessageBox::critical(this, "Ошибка", errMsg);
        } else {
            QMessageBox::information(this, "Успех", "Запись успешно удалена!");
            showSchedule();
        }

        reply->deleteLater();
    });
}

void AdminWindow::setupDateTimeLimits() {
    QDate currentDate = QDate::currentDate();
    ui->dateEdit->setDate(currentDate);
    ui->dateEdit->setMinimumDate(currentDate);
}

void AdminWindow::on_pushButton_add_instructor_clicked() {
    AddInstructorWindow dialog(token, this);
    dialog.exec();
    loadMastersIntoComboBox();
}

void AdminWindow::on_pushButton_fire_instructor_clicked() {
    FireWindow dialog(token, this);
    dialog.exec();
    loadMastersIntoComboBox();
}

void AdminWindow::fillLabels() {
    ui->label_tyreMark->setPixmap(QPixmap(":/assets/labels/tyre_mark.png"));
    ui->label_tyreMark->setScaledContents(true);
    ui->label_tyreMark->setStyleSheet("background: transparent;");
    ui->label_tyreMark_2->setPixmap(QPixmap(":/assets/labels/tyre_mark.png"));
    ui->label_tyreMark_2->setScaledContents(true);
    ui->label_tyreMark_2->setStyleSheet("background: transparent;");
}
