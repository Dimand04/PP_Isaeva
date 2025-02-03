#include "createuser.h"
#include "ui_createuser.h"
#include "global.h"

createuser::createuser(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::createuser)
{
    ui->setupUi(this);
    connect (ui->qpb_createNewAccount,&QPushButton::clicked,this,&createuser::createNewAccount);
    loadRoles();
    setupValidators();
}

createuser::~createuser()
{
    delete ui;
}

//Загрузить роли
void createuser::loadRoles()
{
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_roles->clear();
        query.prepare("SELECT id, role_name FROM roles");
        if(query.exec())
        {
            while (query.next()) {
                int faceId = query.value(0).toInt();
                QString faceName = query.value(1).toString();
                ui->cb_roles->addItem(faceName, faceId);
            }
        }
    }
}

//Создать новый аккаунт
void createuser::createNewAccount()
{
    if (ui->le_newName->text().isEmpty() || ui->le_newSurname->text().isEmpty() || ui->le_newPatronymic->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поля имени, фамилии и отчества не могут быть пустыми!");
        return;
    }

    if (ui->le_newPhone->text().length() != 11) {
        QMessageBox::warning(this, "Ошибка", "Номер телефона должен содержать 11 цифр!");
        return;
    }

    if (ui->le_newPassword->text().isEmpty() || ui->le_newPasswordConfirm->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пароль и подтверждение пароля не могут быть пустыми!");
        return;
    }

    if (ui->le_newPassword->text() != ui->le_newPasswordConfirm->text()) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают! Проверьте введенные данные.");
        return;
    }

    if (ui->cb_roles->currentIndex() == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите роль из списка!");
        return;
    }

    int selectedRoleId = ui->cb_roles->currentData().toInt();
    db = QSqlDatabase::database("db_isaeva");
    if (getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("INSERT INTO users (user_name, user_surname, user_patronymic, user_phone, user_role) VALUES (?, ?, ?, ?, ?);");
        query.addBindValue(ui->le_newName->text());
        query.addBindValue(ui->le_newSurname->text());
        query.addBindValue(ui->le_newPatronymic->text());
        query.addBindValue(ui->le_newPhone->text());
        query.addBindValue(selectedRoleId);

        if (query.exec())
        {
            query.exec("SELECT MAX(id) FROM users;");
            if (query.next())
            {
                userID = query.value(0).toString();
                query.prepare("INSERT INTO authorization (id_users, password) VALUES (?, ?);");
                query.addBindValue(userID);
                query.addBindValue(ui->le_newPassword->text());
                if (query.exec())
                {
                    QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
                    showMessage("Успешная регистрация!", "", QMessageBox::Information, buttons);
                    ui->le_newName->clear();
                    ui->le_newSurname->clear();
                    ui->le_newPatronymic->clear();
                    ui->le_newPhone->clear();
                    ui->le_newPassword->clear();
                    ui->le_newPasswordConfirm->clear();
                    close();
                }
            }
        }
    }
}

void createuser::setupValidators()
{
    QRegularExpression textRegex("^[А-Яа-яA-Za-z ]+$");
    QRegularExpressionValidator *textValidator = new QRegularExpressionValidator(textRegex, this);

    ui->le_newName->setValidator(textValidator);
    ui->le_newSurname->setValidator(textValidator);
    ui->le_newPatronymic->setValidator(textValidator);

    QRegularExpression phoneRegex("^\\d{0,11}$");
    QRegularExpressionValidator *phoneValidator = new QRegularExpressionValidator(phoneRegex, this);

    ui->le_newPhone->setValidator(phoneValidator);
}
