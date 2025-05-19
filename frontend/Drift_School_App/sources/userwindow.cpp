#include "headers/userwindow.h"
#include "ui_userwindow.h"

UserWindow::UserWindow(int userId, const QString& token, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::UserWindow),
      userId(userId),
      token(token),
      networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    setWindowTitle("Ваш аккаунт пользователя");
    setWindowIcon(QIcon(":/assets/icon/icon.JPG"));

    ui->spinBox->setMinimum(1);
    ui->spinBox->setMaximum(5);
    ui->lineEdit_review->setPlaceholderText("Напишите свой отзыв");

    fillLabels();

    showServices();
    showRatings();
    showSchedule();
    showMasters();

    connect(ui->tableWidget_schedule, &QTableWidget::itemChanged, this, &UserWindow::onScheduleItemChanged);
}

UserWindow::~UserWindow()
{
    delete ui;
}

void UserWindow::showServices()
{
    QNetworkRequest request(QUrl("http://localhost:8080/services"));
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray services = doc.array();

        ui->tableWidget->setRowCount(services.size());
        ui->tableWidget->setColumnCount(4);
        ui->tableWidget->setHorizontalHeaderLabels({"Название тарифа", "Описание", "Цена", "Длительность"});

        int row = 0;
        for (const QJsonValue& val : services) {
            QJsonObject obj = val.toObject();
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(obj["service_name"].toString()));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(obj["description"].toString()));
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(obj["price"].toDouble()) + " ₽"));
            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(obj["duration_minutes"].toInt()) + " минут"));
            row++;
        }

        ui->tableWidget->resizeColumnsToContents();
        ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    });
}

void UserWindow::showRatings()
{
    QNetworkRequest request(QUrl("http://localhost:8080/reviews"));
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray reviews = doc.array();

        ui->tableWidget_reviews->setRowCount(reviews.size());
        ui->tableWidget_reviews->setColumnCount(4);
        ui->tableWidget_reviews->setHorizontalHeaderLabels({"Имя клиента", "Оценка", "Комментарий", "Время создания"});

        int row = 0;
        for (const QJsonValue& val : reviews) {
            QJsonObject obj = val.toObject();
            ui->tableWidget_reviews->setItem(row, 0, new QTableWidgetItem(obj["client_name"].toString()));
            ui->tableWidget_reviews->setItem(row, 1, new QTableWidgetItem(QString::number(obj["rating"].toInt())));
            ui->tableWidget_reviews->setItem(row, 2, new QTableWidgetItem(obj["comment"].toString()));

            QDateTime dt = QDateTime::fromString(obj["created_at"].toString(), Qt::ISODate);
            ui->tableWidget_reviews->setItem(row, 3, new QTableWidgetItem(dt.toString("dd.MM.yyyy HH:mm:ss")));

            row++;
        }

        ui->tableWidget_reviews->resizeColumnsToContents();
        ui->tableWidget_reviews->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidget_reviews->setEditTriggers(QAbstractItemView::NoEditTriggers);
    });
}

void UserWindow::showSchedule() {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("http://localhost:8080/schedule"));
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray arr = doc.array();

        disconnect(ui->tableWidget_schedule, &QTableWidget::itemChanged, this, &UserWindow::onScheduleItemChanged);  // отключаем

        ui->tableWidget_schedule->setRowCount(arr.size());
        ui->tableWidget_schedule->setColumnCount(5);
        ui->tableWidget_schedule->setHorizontalHeaderLabels({"Дата", "Инструктор", "Время начала", "Время окончания", "Забронировано"});

        int row = 0;
        for (const QJsonValue &val : arr) {
            QJsonObject obj = val.toObject();

            QDate date = QDate::fromString(obj["available_date"].toString(), "yyyy-MM-dd");
            QTableWidgetItem* dateItem = new QTableWidgetItem(date.toString("dd.MM.yyyy"));
            dateItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            ui->tableWidget_schedule->setItem(row, 0, dateItem);

            QString fullName = obj["first_name"].toString() + " " + obj["last_name"].toString();
            QTableWidgetItem* nameItem = new QTableWidgetItem(fullName);
            nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            ui->tableWidget_schedule->setItem(row, 1, nameItem);

            QString startTime = obj["start_time"].toString().left(5);
            QTableWidgetItem* startItem = new QTableWidgetItem(startTime);
            startItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            ui->tableWidget_schedule->setItem(row, 2, startItem);

            QString endTime = obj["end_time"].toString().left(5);
            QTableWidgetItem* endItem = new QTableWidgetItem(endTime);
            endItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            ui->tableWidget_schedule->setItem(row, 3, endItem);

            // ===== Чекбокс (последний столбец) =====
            bool isBooked = obj["is_booked"].toBool();
            QTableWidgetItem* checkItem = new QTableWidgetItem();
            checkItem->setCheckState(isBooked ? Qt::Checked : Qt::Unchecked);
            checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            int scheduleId = obj["schedule_id"].toInt();
            int ownerUserId = obj.contains("user_id") && !obj["user_id"].isNull()
                ? obj["user_id"].toInt()
                : -1;

            checkItem->setData(Qt::UserRole, scheduleId);
            checkItem->setData(Qt::UserRole + 1, ownerUserId);

            ui->tableWidget_schedule->setItem(row, 4, checkItem);

            row++;
        }


        ui->tableWidget_schedule->resizeColumnsToContents();
        ui->tableWidget_reviews->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableWidget_schedule->setSelectionBehavior(QAbstractItemView::SelectRows);

        connect(ui->tableWidget_schedule, &QTableWidget::itemChanged, this, &UserWindow::onScheduleItemChanged);  // включаем обратно
    });
}

void UserWindow::showMasters()
{
    QNetworkRequest request(QUrl("http://localhost:8080/instructors"));
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonArray arr = doc.array();

        ui->tableWidget_masters->setRowCount(arr.size());
        ui->tableWidget_masters->setColumnCount(4);
        ui->tableWidget_masters->setHorizontalHeaderLabels({"Инструктор", "Специализация", "Телефон", "E-mail"});

        int row = 0;
        for (const QJsonValue& val : arr) {
            QJsonObject obj = val.toObject();
            QString name = obj["first_name"].toString() + " " + obj["last_name"].toString();
            ui->tableWidget_masters->setItem(row, 0, new QTableWidgetItem(name));
            ui->tableWidget_masters->setItem(row, 1, new QTableWidgetItem(obj["specialization"].toString()));
            ui->tableWidget_masters->setItem(row, 2, new QTableWidgetItem(obj["phone_number"].toString()));
            ui->tableWidget_masters->setItem(row, 3, new QTableWidgetItem(obj["email"].toString()));
            row++;
        }

        ui->tableWidget_masters->resizeColumnsToContents();
        ui->tableWidget_masters->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidget_masters->setEditTriggers(QAbstractItemView::NoEditTriggers);
    });
}

void UserWindow::onScheduleItemChanged(QTableWidgetItem *item)
{
    if (item->column() != 4) return;

    // Снимаем сигнал, чтобы не было рекурсии
    disconnect(ui->tableWidget_schedule, &QTableWidget::itemChanged, this, &UserWindow::onScheduleItemChanged);

    bool isBooked = (item->checkState() == Qt::Checked);
    int scheduleId = item->data(Qt::UserRole).toInt();
    int ownerUserId = item->data(Qt::UserRole + 1).toInt();

    // Проверка: бронирует не владелец
    if (isBooked && ownerUserId != -1 && ownerUserId != this->userId) {
        QMessageBox::warning(this, "Ошибка", "Слот уже забронирован другим пользователем.");
        item->setCheckState(Qt::Unchecked);
        connect(ui->tableWidget_schedule, &QTableWidget::itemChanged, this, &UserWindow::onScheduleItemChanged);
        return;
    }

    // Проверка: отменяет чужую бронь
    if (!isBooked && ownerUserId != this->userId) {
        QMessageBox::warning(this, "Ошибка", "Вы не можете отменить чужую бронь.");
        item->setCheckState(Qt::Checked);
        connect(ui->tableWidget_schedule, &QTableWidget::itemChanged, this, &UserWindow::onScheduleItemChanged);
        return;
    }

    // Отправляем PATCH-запрос
    QJsonObject payload;
    payload["schedule_id"] = scheduleId;
    payload["book"] = isBooked;

    QNetworkRequest request(QUrl("http://localhost:8080/schedule/book"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = networkManager->sendCustomRequest(request, "PATCH", QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj.contains("error")) {
            QMessageBox::critical(this, "Ошибка", obj["error"].toString());
            item->setCheckState(isBooked ? Qt::Unchecked : Qt::Checked);
        } else {
            showSchedule();  // Обновим всю таблицу
        }

        connect(ui->tableWidget_schedule, &QTableWidget::itemChanged, this, &UserWindow::onScheduleItemChanged);  // включаем обратно
    });
}

void UserWindow::createOrder(int userId, int instructorId, int serviceId, const QString &orderDate, double totalPrice)
{
    QJsonObject payload;
    payload["user_id"] = userId;
    payload["instructor_id"] = instructorId;
    payload["service_id"] = serviceId;
    payload["order_date"] = orderDate;
    payload["total_price"] = totalPrice;

    QNetworkRequest request(QUrl("http://localhost:8080/orders"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = networkManager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj.contains("error")) {
            QMessageBox::critical(this, "Ошибка", obj["error"].toString());
        } else {
            qDebug() << "Заказ успешно создан!";
        }
    });
}

void UserWindow::deleteOrder(int userId, int instructorId, const QString &orderDate)
{
    QJsonObject payload;
    payload["user_id"] = userId;
    payload["instructor_id"] = instructorId;
    payload["order_date"] = orderDate;

    QNetworkRequest request(QUrl("http://localhost:8080/orders/delete"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = networkManager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj.contains("error")) {
            QMessageBox::critical(this, "Ошибка", obj["error"].toString());
        } else {
            qDebug() << "Заказ успешно удалён!";
        }
    });
}

void UserWindow::on_pushButton_review_clicked()
{
    int rating = ui->spinBox->value();
    QString comment = ui->lineEdit_review->text().trimmed();

    if (comment.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, напишите комментарий.");
        return;
    }

    QJsonObject payload;
    payload["rating"] = rating;
    payload["comment"] = comment;

    QNetworkRequest request(QUrl("http://localhost:8080/reviews"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    QNetworkReply* reply = networkManager->post(request, QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();

        if (obj.contains("error")) {
            QMessageBox::critical(this, "Ошибка", obj["error"].toString());
        } else {
            QMessageBox::information(this, "Спасибо", "Ваш отзыв был успешно добавлен!");
            ui->lineEdit_review->clear();
            ui->spinBox->setValue(1);
            showRatings();  // обновим таблицу
        }
    });
}

void UserWindow::fillLabels()
{
    QMovie *movie = new QMovie(":/assets/labels/Like.gif");
    ui->label_like->setMovie(movie);
    ui->label_like->setScaledContents(true);
    movie->start();
    ui->label_star->setPixmap(QPixmap(":/assets/labels/star.png"));
    ui->label_star->setScaledContents(true);
    ui->label_star->setStyleSheet("background: transparent;");
}

void UserWindow::on_pushButton_exit_clicked()
{
    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();
    this->close();
}

