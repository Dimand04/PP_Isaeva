#ifndef CREATERECEIPT_H
#define CREATERECEIPT_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>

namespace Ui {
class createreceipt;
}

class createreceipt : public QWidget
{
    Q_OBJECT

signals:
    void receiptAdded();

public:
    explicit createreceipt(const QString& userID, QWidget *parent = nullptr);
    ~createreceipt();

public slots:
    void addNewReceipt();
    void loadMaterials();
    void showMaterialMenu();
    void showMainMenu();
    void addNewMaterial();
    void selectDoc();
    void sumStorage();

private:
    Ui::createreceipt *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "db_isaeva");
    QPixmap getImageFromDB(QByteArray);
    QString userID;
};

#endif // CREATERECEIPT_H
