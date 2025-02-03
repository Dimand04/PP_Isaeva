#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "createuser.h"
#include "createconsumtion.h"
#include "createreceipt.h"
#include <QDateTime>
#include "doc.h"
#include <QRegularExpressionValidator>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect (ui->qpb_login,&QPushButton::clicked,this,&MainWindow::tryLogin);
    connect (ui->qpb_logout,&QPushButton::clicked,this,&MainWindow::logout);
    connect (ui->qpb_p_change,&QPushButton::clicked,this,&MainWindow::lereadonly);
    connect (ui->qpb_p_change_ok,&QPushButton::clicked,this,&MainWindow::updateUserInfo);
    connect (ui->qpb_adduser,&QPushButton::clicked,this,&MainWindow::createNewAccount);
    connect (ui->qpb_addReceipt,&QPushButton::clicked,this,&MainWindow::createNewReceipt);
    connect(ui->tw_receipt, &QTableWidget::cellClicked, this, &MainWindow::TableWidgetReceiptRowClicked);
    connect(ui->tw_consumtion, &QTableWidget::cellClicked, this, &MainWindow::TableWidgetConsumptionRowClicked);
    connect (ui->qpb_addConsumption,&QPushButton::clicked,this,&MainWindow::createNewConsumption);

    ui->tw_receipt->setColumnWidth(0,200);
    ui->tw_receipt->setColumnWidth(1,557);

    ui->tw_consumtion->setColumnWidth(0,200);
    ui->tw_consumtion->setColumnWidth(1,557);

    ui->tw_storage->setColumnWidth(0,557);

    setupValidators();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//Показать меню авторизации
void MainWindow::showLoginMenu()
{
    ui->frame_login->setGeometry(ui->frame_login->x(), 0, ui->frame_login->width(), ui->frame_login->height());
    ui->frame_main->setGeometry(ui->frame_main->x(), 1000, ui->frame_main->width(), ui->frame_main->height());
    ui->frame_receipt->setGeometry(ui->frame_receipt->x(), 1000, ui->frame_receipt->width(), ui->frame_receipt->height());
    ui->frame_consumption->setGeometry(ui->frame_consumption->x(), 1000, ui->frame_consumption->width(), ui->frame_consumption->height());
    ui->frame_login->setEnabled(true);
    ui->frame_main->setEnabled(false);
    ui->frame_receipt->setEnabled(false);
    ui->frame_consumption->setEnabled(false);
}

//Показать главное меню
void MainWindow::showMainMenu()
{
    ui->frame_login->setGeometry(ui->frame_login->x(), 1000, ui->frame_login->width(), ui->frame_login->height());
    ui->frame_main->setGeometry(ui->frame_main->x(), 0, ui->frame_main->width(), ui->frame_main->height());
    ui->frame_login->setEnabled(false);
    ui->frame_main->setEnabled(true);
    ui->le_p_name->setReadOnly(true);
    ui->le_p_surname->setReadOnly(true);
    ui->le_p_patronymic->setReadOnly(true);
    ui->le_p_phone->setReadOnly(true);
    ui->qpb_p_change_ok->setVisible(false);
    getUserInfo(userID);
    loadReceiptTable();
    loadConsumptionTable();
    loadStorageTable();
}

//Вывести фрейм поступлений
void MainWindow::showReceiptMenu()
{
    ui->frame_receipt->setGeometry(ui->frame_receipt->x(), 29, ui->frame_receipt->width(), ui->frame_receipt->height());
    ui->frame_consumption->setGeometry(ui->frame_consumption->x(), 1000, ui->frame_consumption->width(), ui->frame_consumption->height());
    ui->frame_receipt->setEnabled(true);
    ui->frame_consumption->setEnabled(false);
}

//Вывести фрейм вычетов
void MainWindow::showConsumptionMenu()
{
    ui->frame_receipt->setGeometry(ui->frame_receipt->x(), 1000, ui->frame_receipt->width(), ui->frame_receipt->height());
    ui->frame_consumption->setGeometry(ui->frame_consumption->x(), 29, ui->frame_consumption->width(), ui->frame_consumption->height());
    ui->frame_receipt->setEnabled(false);
    ui->frame_consumption->setEnabled(true);
}

//Попытка авторизации
void MainWindow::tryLogin()
{
    QString login = ui->le_login->text().remove(" ");
    QString password = ui->le_password->text().remove(" ");

    if (login.isEmpty() && password.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните поля 'Номер телефона' и 'Пароль'.");
        return;
    }
    else if (login.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните поле 'Номер телефона'.");
        return;
    }
    else if (password.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните поле 'Пароль'.");
        return;
    }

    db = QSqlDatabase::database("db_isaeva");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("SELECT id, user_role FROM users INNER JOIN authorization ON users.id = authorization.id_users WHERE users.user_phone = ? AND authorization.password = ?;");
        query.addBindValue(login);
        query.addBindValue(password);
        query.exec();
        if(query.next())
        {
            QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
            showMessage("Успешная авторизация!", "Успех", QMessageBox::Information, buttons);
            userID = query.value(0).toString();
            userRole = query.value(1).toString();
            ui->le_login->clear();
            ui->le_password->clear();
            showMainMenu();
            adminFunction();
        }
        else
        {
            QMessageBox::warning(this, "Ошибка", "Пользователя с такими данными не существует!");
            ui->le_login->clear();
            ui->le_password->clear();
            return;
        }
    }
}

//Получение роли пользователя
bool MainWindow::hasAccess(const QString &requiredRole)
{
    return userRole == requiredRole;
    qDebug() << userRole;
}

//Определение прав пользователя
void MainWindow::adminFunction()
{
    if(hasAccess("2"))
    {
        ui->qpb_adduser->setVisible(false);
    }
    else
    {
        ui->qpb_adduser->setVisible(true);
    }
}

//Выход из учётки
void MainWindow::logout()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение выхода", "Вы действительно хотите выйти из учётной записи?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        userID.clear();
        userRole.clear();
        showLoginMenu();
    }
}

//Загрузка информации пользователя
void MainWindow::getUserInfo(QString ID)
{
    db = QSqlDatabase::database("db_isaeva");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("select user_name, user_surname, user_patronymic, user_phone from users where id = ?;");
        query.addBindValue(ID);
        query.exec();
        if(query.next())
        {
            ui->le_p_name->setText(query.value(0).toString());
            ui->le_p_surname->setText(query.value(1).toString());
            ui->le_p_patronymic->setText(query.value(2).toString());
            ui->le_p_phone->setText(query.value(3).toString());
        }
    }
}

//Проверка элементов
void MainWindow::lereadonly()
{
    bool isReadOnly = ui->le_p_name->isReadOnly();
    if (!isReadOnly) {
        getUserInfo(userID);
    }
    ui->le_p_name->setReadOnly(!isReadOnly);
    ui->le_p_surname->setReadOnly(!isReadOnly);
    ui->le_p_patronymic->setReadOnly(!isReadOnly);
    ui->le_p_phone->setReadOnly(!isReadOnly);
    ui->qpb_p_change_ok->setVisible(true);
    if (isReadOnly) {
        ui->qpb_p_change->setText("Отмена");
    } else {
        ui->qpb_p_change->setText("Изменить");
        ui->qpb_p_change_ok->setVisible(false);
    }
}

//Обновить информацию пользователя
void MainWindow::updateUserInfo()
{
    if (ui->le_p_phone->text().length() != 11) {
        QMessageBox::warning(this, "Ошибка", "Номер телефона может содержать только 11 цифр!");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение изменений", "Вы действительно хотите изменить данные?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (ui->le_p_name->isReadOnly()) return;
        db = QSqlDatabase::database("db_isaeva");
        if (getDBConnection(db))
        {
            QSqlQuery query(db);
            query.prepare("UPDATE users SET user_name = ?, user_surname = ?, user_patronymic = ?, user_phone = ? WHERE id = ?");
            query.addBindValue(ui->le_p_name->text());
            query.addBindValue(ui->le_p_surname->text());
            query.addBindValue(ui->le_p_patronymic->text());
            query.addBindValue(ui->le_p_phone->text());
            query.addBindValue(userID);
            if (query.exec())
            {
                QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
                showMessage("Данные обновлены!", "Успех", QMessageBox::Information, buttons);
                lereadonly();
                getUserInfo(userID);
            }
        }
    }
}

//Вызвать форму создания нового аккаунта
void MainWindow::createNewAccount()
{
    createuser *cnu = new createuser;
    cnu->setWindowTitle("Создание учётной записи");
    cnu->show();
}

//Загрузить таблицу поступлений
void MainWindow::loadReceiptTable()
{
    db = QSqlDatabase::database("db_isaeva");
    if(getDBConnection(db))
    {
        int row = 0;
        QSqlQuery query(db);

        query.prepare("SELECT id, receipt_date FROM receipt");

        if(query.exec())
        {
            ui->tw_receipt->setRowCount(0);
            while(query.next())
            {
                ui->tw_receipt->insertRow(row);

                int receiptId = query.value(0).toInt();
                QDateTime receiptDateTime = query.value(1).toDateTime();

                QString formattedDate = receiptDateTime.toString("yyyy-MM-dd HH:mm:ss");

                QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(receiptId));
                QTableWidgetItem *dateItem = new QTableWidgetItem(formattedDate);

                ui->tw_receipt->setItem(row, 0, idItem);
                ui->tw_receipt->setItem(row, 1, dateItem);
                row++;
            }
        }
    }
}

//Загрузить таблицу вычетов
void MainWindow::loadConsumptionTable()
{
    db = QSqlDatabase::database("db_isaeva");
    if(getDBConnection(db))
    {
        int row = 0;
        QSqlQuery query(db);

        query.prepare("SELECT id, writeoff_date FROM writeoff");

        if(query.exec())
        {
            ui->tw_consumtion->setRowCount(0);
            while(query.next())
            {
                ui->tw_consumtion->insertRow(row);

                int receiptId = query.value(0).toInt();
                QDateTime receiptDateTime = query.value(1).toDateTime();

                QString formattedDate = receiptDateTime.toString("yyyy-MM-dd HH:mm:ss");

                QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(receiptId));
                QTableWidgetItem *dateItem = new QTableWidgetItem(formattedDate);

                ui->tw_consumtion->setItem(row, 0, idItem);
                ui->tw_consumtion->setItem(row, 1, dateItem);
                row++;
            }
        }
    }
}

//Вызвать форму создания записи поступлений
void MainWindow::createNewReceipt()
{
    createreceipt *cnr = new createreceipt(userID);
    cnr->setWindowTitle("Создание нового поступления");
    cnr->setWindowFlags(Qt::Window);
    cnr->show();
    connect(cnr, &createreceipt::receiptAdded, this, [=]()
            {
                cnr->close();
                loadReceiptTable();
                loadStorageTable();
            });
}

//Вызвать форму создания записи вычета
void MainWindow::createNewConsumption()
{
    createconsumtion *cnc = new createconsumtion(userID);
    cnc->setWindowTitle("Создание нового расхода");
    cnc->setWindowFlags(Qt::Window);
    cnc->show();
    connect(cnc, &createconsumtion::consumptionAdded, this, [=]()
            {
                cnc->close();
                loadConsumptionTable();
                loadStorageTable();
            });
}

//Клик на таблицу с поступлениями
void MainWindow::TableWidgetReceiptRowClicked(int row, int column)
{
    QTableWidgetItem *item = ui->tw_receipt->item(row, 0);
    if (item)
    {
        int orderId = item->text().toInt();
        loadReceiptDetails(orderId);
    }
}

//Клик на таблицу с вычетами
void MainWindow::TableWidgetConsumptionRowClicked(int row, int column)
{
    QTableWidgetItem *item = ui->tw_consumtion->item(row, 0);
    if (item)
    {
        int orderId = item->text().toInt();
        loadConsumtionDetails(orderId);
    }
}

//Загрузить даныне поступления
void MainWindow::loadReceiptDetails(int orderId)
{
    showReceiptMenu();
    currentOrderId = orderId;
    disconnect(ui->lb_image, &QPushLabel::clicked, nullptr, nullptr);
    db = QSqlDatabase::database("db_isaeva");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("select receipt.id, receipt.receipt_date, receipt.description, receipt.document, users.user_name, users.user_patronymic, users.user_surname "
                      "from receipt "
                      "inner join users ON receipt.author_id = users.id "
                      "where receipt.id = :orderID");
        query.bindValue(":orderID", orderId);
        if(query.exec())
        {
            while(query.next())
            {
                QString receiptId = query.value(0).toString();
                QDateTime receiptDateTime = query.value(1).toDateTime();
                QString receiptDate = receiptDateTime.toString("yyyy-MM-dd HH:mm:ss");
                QString receiptName = query.value(4).toString();
                QString receiptPatronymic = query.value(5).toString();
                QString receiptSurname = query.value(6).toString();
                QString fullname = receiptName + " " + receiptPatronymic + " " + receiptSurname;
                QString receiptDescription = query.value(2).toString();
                QByteArray receiptDocument = query.value(3).toByteArray();
                ui->lb_number->setText(receiptId);
                ui->lb_date->setText(receiptDate);
                ui->lb_author->setText(fullname);
                ui->pte_description->setPlainText(receiptDescription);
                QImage image;
                image.loadFromData(receiptDocument);
                ui->lb_image->setPixmap(QPixmap::fromImage(image));
            }
        }
    }
    connect(ui->lb_image, &QPushLabel::clicked, this, [this, orderId]() { loadDoc(orderId); });
}

//Загрузить данные вычета
void MainWindow::loadConsumtionDetails(int orderId)
{
    showConsumptionMenu();
    currentOrderId = orderId;

    disconnect(ui->lb_image_2, &QPushLabel::clicked, nullptr, nullptr);

    db = QSqlDatabase::database("db_isaeva");
    if (getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("SELECT writeoff.id, "
                      "       writeoff.writeoff_date, "
                      "       writeoff.description, "
                      "       writeoff.document, "
                      "       users.user_name, "
                      "       users.user_patronymic, "
                      "       users.user_surname, "
                      "       materials.material_name, "
                      "       writeoff.material_count "
                      "FROM writeoff "
                      "INNER JOIN users ON writeoff.author_id = users.id "
                      "INNER JOIN materials ON writeoff.material_id = materials.id "
                      "WHERE writeoff.id = :orderID");
        query.bindValue(":orderID", orderId);
        if (query.exec())
        {
            while (query.next())
            {
                QString writeoffId = query.value(0).toString();
                QDateTime writeoffDateTime = query.value(1).toDateTime();
                QString writeoffDate = writeoffDateTime.toString("yyyy-MM-dd HH:mm:ss");
                QString userName = query.value(4).toString();
                QString userPatronymic = query.value(5).toString();
                QString userSurname = query.value(6).toString();
                QString fullname = userName + " " + userPatronymic + " " + userSurname;
                QString writeoffDescription = query.value(2).toString();
                QByteArray writeoffDocument = query.value(3).toByteArray();
                QString materialName = query.value(7).toString();
                QString materialCount = query.value(8).toString();

                // Установка данных в элементы интерфейса
                ui->lb_number_2->setText(writeoffId);
                ui->lb_date_2->setText(writeoffDate);
                ui->lb_author_2->setText(fullname);
                ui->pte_description_2->setPlainText(writeoffDescription);
                QImage image;
                image.loadFromData(writeoffDocument);
                ui->lb_image_2->setPixmap(QPixmap::fromImage(image));
                ui->lb_material_2->setText(materialName);
                ui->lb_count_2->setText(materialCount);
            }
        }
    }
    connect(ui->lb_image_2, &QPushLabel::clicked, this, [this, orderId]() { loadDoc(orderId); });
}

//Вызвать форму документа
void MainWindow::loadDoc(int orderID)
{
    doc *chp = new doc(orderID);
    chp->setWindowTitle("Документ");
    chp->show();
}

//Загрузить данные склада
void MainWindow::loadStorageTable()
{
    db = QSqlDatabase::database("db_isaeva");
    if(getDBConnection(db))
    {
        int row = 0;
        QSqlQuery query(db);

        query.prepare("SELECT storage.material_count, storage.material_price, materials.material_name FROM storage "
                      "INNER JOIN materials ON storage.material_id = materials.id");

        if (query.exec())
        {
            ui->tw_storage->setRowCount(0);
            while (query.next())
            {
                ui->tw_storage->insertRow(row);

                QString material_name = query.value(2).toString();
                int material_count = query.value(0).toInt();
                int material_price = query.value(1).toInt();

                ui->tw_storage->setItem(row, 0, new QTableWidgetItem(material_name));
                ui->tw_storage->setItem(row, 1, new QTableWidgetItem(QString::number(material_count)));
                ui->tw_storage->setItem(row, 2, new QTableWidgetItem(QString::number(material_price)));

                row++;
            }
        }
    }
}

void MainWindow::setupValidators()
{
    QRegularExpression textRegex("^[А-Яа-яA-Za-z ]+$");
    QRegularExpressionValidator *textValidator = new QRegularExpressionValidator(textRegex, this);

    ui->le_p_name->setValidator(textValidator);
    ui->le_p_surname->setValidator(textValidator);
    ui->le_p_patronymic->setValidator(textValidator);

    QRegularExpression phoneRegex("^\\d{0,11}$");
    QRegularExpressionValidator *phoneValidator = new QRegularExpressionValidator(phoneRegex, this);

    ui->le_p_phone->setValidator(phoneValidator);
}
