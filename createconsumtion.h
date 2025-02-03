#ifndef CREATECONSUMTION_H
#define CREATECONSUMTION_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>

namespace Ui {
class createconsumtion;
}

class createconsumtion : public QWidget
{
    Q_OBJECT

signals:
    void consumptionAdded();

public:
    explicit createconsumtion(const QString& userID, QWidget *parent = nullptr);
    ~createconsumtion();

public slots:
    void addNewConsumption();
    void loadMaterials();
    void selectDoc();
    void subtractStorage();

private:
    Ui::createconsumtion *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "db_isaeva");
    QPixmap getImageFromDB(QByteArray);
    QString userID;
};

#endif // CREATECONSUMTION_H
