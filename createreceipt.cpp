#include "createreceipt.h"
#include "ui_createreceipt.h"
#include "global.h"
#include <QDateTime>
#include <QPixmap>
#include <QImage>
#include <QBuffer>
#include <QFileDialog>

createreceipt::createreceipt(const QString& userID, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::createreceipt)
    , userID(userID)
{
    ui->setupUi(this);

    connect (ui->qpb_addNewreceipt,&QPushButton::clicked,this,&createreceipt::addNewReceipt);
    connect (ui->qpb_material,&QPushButton::clicked,this,&createreceipt::showMaterialMenu);
    connect (ui->qpb_addMaterialCancel,&QPushButton::clicked,this,&createreceipt::showMainMenu);
    connect (ui->qpb_addNewmaterial,&QPushButton::clicked,this,&createreceipt::addNewMaterial);
    connect (ui->lb_image,&QPushLabel::clicked,this,&createreceipt::selectDoc);

    loadMaterials();
}

createreceipt::~createreceipt()
{
    delete ui;
}

//Добавить новое поступление
void createreceipt::addNewReceipt()
{
    if (ui->sb_count->value() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Количество материала должно быть не меньше 1!");
        return;
    }

    if (ui->dsb_price->value() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Цена материала должна быть не меньше 1 условной единицы!");
        return;
    }

    db = QSqlDatabase::database("db_isaeva");
    if(getDBConnection(db))
    {
        QPixmap image = ui->lb_image->pixmap();
        QByteArray arr;
        QBuffer buffer(&arr);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        QSqlQuery query(db);

        QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        query.prepare("INSERT INTO receipt (receipt_date, author_id, description, document) VALUES (:receipt_date, :author_id, :description, :document)");
        query.bindValue(":receipt_date", currentDateTime);
        query.bindValue(":author_id", userID);
        query.bindValue(":description", ui->pte_description->toPlainText());
        query.bindValue(":document", arr);

        if (query.exec())
        {
            sumStorage();
            QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
            showMessage("Заявка добавлена, материал занесен в склад!", "Успех", QMessageBox::Information, buttons);
            emit receiptAdded();
        }
        else
        {
            showMessage("Ошибка при добавлении заявки: ", "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
        }
    }
}


//Загрузить материалы
void createreceipt::loadMaterials()
{
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_materials->clear();
        query.prepare("SELECT id, material_name FROM materials");
        if(query.exec())
        {
            while (query.next()) {
                int faceId = query.value(0).toInt();
                QString faceName = query.value(1).toString();
                ui->cb_materials->addItem(faceName, faceId);
            }
        }
    }
}

//Показать меню материалов
void createreceipt::showMaterialMenu()
{
    ui->frame_material->setGeometry(ui->frame_material->x(), 0, ui->frame_material->width(), ui->frame_material->height());
    ui->frame->setGeometry(ui->frame->x(), 1000, ui->frame->width(), ui->frame->height());
    ui->frame_material->setEnabled(true);
    ui->frame->setEnabled(false);
}

//Показать галвное меню
void createreceipt::showMainMenu()
{
    ui->frame_material->setGeometry(ui->frame_material->x(), 1000, ui->frame_material->width(), ui->frame_material->height());
    ui->frame->setGeometry(ui->frame->x(), 0, ui->frame->width(), ui->frame->height());
    ui->frame_material->setEnabled(false);
    ui->frame->setEnabled(true);
}

//Добавить новый материал
void createreceipt::addNewMaterial()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение", "Вы уверены, что хотите добавить материал?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        db = QSqlDatabase::database("db_isaeva");
        if(getDBConnection(db))
        {
            QSqlQuery query(db);
            query.prepare("insert into materials (material_name) values (?);");
            query.addBindValue(ui->le_materialName->text());
            if(query.exec())
            {
                QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
                showMessage("Материал успешно добавлен!", "", QMessageBox::Information, buttons);
                ui->le_materialName->clear();
                loadMaterials();
                showMainMenu();
            }
        }
    }
}

//Получить изображение
QPixmap createreceipt::getImageFromDB(QByteArray arr)
{
    QPixmap image;
    image.loadFromData(arr);
    return image;
}

//Выбрать документ
void createreceipt::selectDoc()
{
    QStringList imageName;
    QFileDialog fd;
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::Detail);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle("Выберите документ");
    fd.setNameFilter("Image files (*.png *.jpeg *.jpg)");
    if(fd.exec())
    {
        imageName = fd.selectedFiles();
    }
    if(!imageName.isEmpty())
    {
        QPixmap image;
        image.load(imageName.at(0));
        ui->lb_image->setPixmap(image);
    }
}

void createreceipt::sumStorage()
{
    db = QSqlDatabase::database("db_isaeva");
    if(getDBConnection(db))
    {
        int materialId = ui->cb_materials->currentData().toInt();
        int newCount = ui->sb_count->value();
        double newPrice = ui->dsb_price->value();

        QSqlQuery query(db);

        // Проверяем, существует ли запись для данного материала
        query.prepare("SELECT material_count, material_price FROM storage WHERE material_id = :material_id");
        query.bindValue(":material_id", materialId);

        if (query.exec())
        {
            if (query.next())
            {
                int currentCount = query.value("material_count").toInt();
                double currentPrice = query.value("material_price").toDouble();

                int updatedCount = currentCount + newCount;
                double updatedPrice = (currentPrice * currentCount + newPrice * newCount) / updatedCount;

                QSqlQuery updateQuery(db);
                updateQuery.prepare("UPDATE storage SET material_count = :material_count, material_price = :material_price WHERE material_id = :material_id");
                updateQuery.bindValue(":material_count", updatedCount);
                updateQuery.bindValue(":material_price", updatedPrice);
                updateQuery.bindValue(":material_id", materialId);

                if (!updateQuery.exec())
                {
                    showMessage("Ошибка при обновлении данных в storage: ",
                                "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
                }
            }
            else
            {
                QSqlQuery insertQuery(db);
                insertQuery.prepare("INSERT INTO storage (material_id, material_count, material_price) VALUES (:material_id, :material_count, :material_price)");
                insertQuery.bindValue(":material_id", materialId);
                insertQuery.bindValue(":material_count", newCount);
                insertQuery.bindValue(":material_price", newPrice);

                if (!insertQuery.exec())
                {
                    showMessage("Ошибка при добавлении данных в storage: ",
                                "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
                }
            }
        }
    }
}
