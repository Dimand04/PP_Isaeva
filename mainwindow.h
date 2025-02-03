#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showLoginMenu();
    void showMainMenu();
    void showReceiptMenu();
    void showConsumptionMenu();
    void tryLogin();
    void logout();
    void lereadonly();
    void adminFunction();
    void loadReceiptTable();
    void loadConsumptionTable();
    void createNewReceipt();
    void createNewConsumption();
    void TableWidgetReceiptRowClicked(int row, int column);
    void TableWidgetConsumptionRowClicked(int row, int column);
    void loadReceiptDetails(int orderId);
    void loadConsumtionDetails(int orderId);
    void loadStorageTable();
    void setupValidators();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "db_isaeva");
    QString userID;
    QString userRole;
    int currentOrderId;
    void loadDoc(int ID);
    void getUserInfo(QString);
    void updateUserInfo();
    void createNewAccount();
    bool hasAccess(const QString &requiredRole);
};
#endif // MAINWINDOW_H
