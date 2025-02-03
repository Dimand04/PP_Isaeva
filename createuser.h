#ifndef CREATEUSER_H
#define CREATEUSER_H

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>

namespace Ui {
class createuser;
}

class createuser : public QWidget
{
    Q_OBJECT

public:
    explicit createuser(QWidget *parent = nullptr);
    ~createuser();

public slots:
    void loadRoles();
    void createNewAccount();
    void setupValidators();

private:
    Ui::createuser *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "db_isaeva");
    QString userID;
};

#endif // CREATEUSER_H
