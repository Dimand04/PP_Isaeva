#include "createconsumtion.h"
#include "ui_createconsumtion.h"
#include "global.h"
#include <QDateTime>
#include <QPixmap>
#include <QImage>
#include <QBuffer>
#include <QFileDialog>

createconsumtion::createconsumtion(const QString& userID, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::createconsumtion)
    , userID(userID)
{
    ui->setupUi(this);

    connect (ui->qpb_addNewreceipt,&QPushButton::clicked,this,&createconsumtion::addNewConsumption);
    connect (ui->lb_image,&QPushLabel::clicked,this,&createconsumtion::selectDoc);

    loadMaterials();
}

createconsumtion::~createconsumtion()
{
    delete ui;
}

void createconsumtion::addNewConsumption()
{
    if (ui->sb_count->value() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Количество материала должно быть не меньше 1!");
        return;
    }

    db = QSqlDatabase::database("db_isaeva");
    if (getDBConnection(db))
    {
        int materialId = ui->cb_materials->currentData().toInt();
        int deductCount = ui->sb_count->value();

        // Проверяем, достаточно ли материала на складе
        QSqlQuery checkQuery(db);
        checkQuery.prepare("SELECT material_count FROM storage WHERE material_id = :material_id");
        checkQuery.bindValue(":material_id", materialId);

        if (checkQuery.exec())
        {
            if (checkQuery.next())
            {
                int currentCount = checkQuery.value("material_count").toInt();
                if (currentCount < deductCount)
                {
                    showMessage("На складе недостаточно материала для вычета.", "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
                    return;
                }
            }
            else
            {
                showMessage("Материал с указанным ID отсутствует на складе.", "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
                return;
            }
        }
        else
        {
            showMessage("Ошибка при выполнении запроса к базе данных.", "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
            return;
        }

        // Если материала достаточно, продолжаем выполнение
        QPixmap image = ui->lb_image->pixmap();
        QByteArray arr;
        QBuffer buffer(&arr);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");

        QSqlQuery query(db);
        QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        query.prepare("INSERT INTO writeoff (writeoff_date, author_id, description, document, material_id, material_count) "
                      "VALUES (:writeoff_date, :author_id, :description, :document, :material_id, :material_count)");
        query.bindValue(":writeoff_date", currentDateTime);
        query.bindValue(":author_id", userID);
        query.bindValue(":description", ui->pte_description->toPlainText());
        query.bindValue(":document", arr);
        query.bindValue(":material_id", materialId);
        query.bindValue(":material_count", deductCount);

        if (query.exec())
        {
            subtractStorage();
            QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
            showMessage("Запись о вычете добавлена, материалы вычтены со склада!", "Успех", QMessageBox::Information, buttons);
            emit consumptionAdded();
        }
        else
        {
            showMessage("Ошибка при добавлении записи о вычете: ", "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
        }
    }
}


void createconsumtion::loadMaterials()
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

void createconsumtion::selectDoc()
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

void createconsumtion::subtractStorage()
{
    db = QSqlDatabase::database("db_isaeva");
    if (getDBConnection(db))
    {
        int materialId = ui->cb_materials->currentData().toInt();
        int deductCount = ui->sb_count->value();

        QSqlQuery query(db);

        query.prepare("SELECT material_count FROM storage WHERE material_id = :material_id");
        query.bindValue(":material_id", materialId);

        if (query.exec())
        {
            if (query.next())
            {
                int currentCount = query.value("material_count").toInt();

                if (currentCount >= deductCount)
                {
                    int updatedCount = currentCount - deductCount;

                    QSqlQuery updateQuery(db);
                    updateQuery.prepare("UPDATE storage SET material_count = :material_count WHERE material_id = :material_id");
                    updateQuery.bindValue(":material_count", updatedCount);
                    updateQuery.bindValue(":material_id", materialId);

                    if (updateQuery.exec())
                    {
                        QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
                        showMessage("Материал успешно вычтен со склада!", "Успех", QMessageBox::Information, buttons);
                        emit consumptionAdded();
                    }
                    else
                    {
                        showMessage("Ошибка при обновлении данных на складе: ", "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
                    }
                }
                else
                {
                    showMessage("На складе недостаточно материала для вычета.", "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
                }
            }
            else
            {
                showMessage("Материал с указанным ID отсутствует на складе.", "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
            }
        }
        else
        {
            showMessage("Ошибка при выполнении запроса к базе данных.", "Ошибка", QMessageBox::Critical, {QMessageBox::Ok});
        }
    }
}
