#include "UI_Dialogs.h"
#include <qregularexpression.h>
#include "QRegularExpression"
#include "QDate"

searchAddressBookItemDialog::searchAddressBookItemDialog(QWidget *parent) : QDialog(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("Введите текст для поиска");

    searchButton = new QPushButton("Поиск", this);
    cancelButton = new QPushButton("Отмена", this);

    connect(searchButton, &QPushButton::clicked, this, &searchAddressBookItemDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &searchAddressBookItemDialog::reject);

    mainLayout->addWidget(new QLabel("Поиск по контактам:", this));
    mainLayout->addWidget(searchInput);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(searchButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

QString searchAddressBookItemDialog::getSearchTerm() const {
    return searchInput->text().trimmed();
}







addAddressBookItemDialog::addAddressBookItemDialog(QWidget *parent) : QDialog(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    userLastNameInput = new QLineEdit(this);
    userFirstNameInput = new QLineEdit(this);
    userPatronymicNameInput = new QLineEdit(this);
    phoneInput = new QLineEdit(this);
    userEmailInput = new QLineEdit(this);
    userBirthdayInput = new QLineEdit(this);

    userLastNameInput->setPlaceholderText("Фамилия");
    userLastNameInput->setStyleSheet("padding: 5px; border:solid");

    userFirstNameInput->setPlaceholderText("Имя");
    userFirstNameInput->setStyleSheet("padding: 5px; border:solid");

    userPatronymicNameInput->setPlaceholderText("Отчество");
    userPatronymicNameInput->setStyleSheet("padding: 5px; border:solid");

    phoneInput->setPlaceholderText("Телефон");
    phoneInput->setStyleSheet("padding: 5px; border:solid");

    userEmailInput->setPlaceholderText("E-mail");
    userEmailInput->setStyleSheet("padding: 5px; border:solid");

    userBirthdayInput->setPlaceholderText("Дата рождения (DD-MM-YYYY)");
    userBirthdayInput->setStyleSheet("padding: 5px; border:solid");

    addButton = new QPushButton("Добавить", this);
    cancelButton = new QPushButton("Отмена", this);

    connect(addButton, &QPushButton::clicked, this, [this]() {
        if (validateInput()) accept();
    });

    connect(cancelButton, &QPushButton::clicked, this, &addAddressBookItemDialog::reject);

    mainLayout->addWidget(userLastNameInput);
    mainLayout->addWidget(userFirstNameInput);
    mainLayout->addWidget(userPatronymicNameInput);
    mainLayout->addWidget(phoneInput);
    mainLayout->addWidget(userEmailInput);
    mainLayout->addWidget(userBirthdayInput);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

QStringList addAddressBookItemDialog::getItem() const {
    return {userLastNameInput->text(), userFirstNameInput->text(), userPatronymicNameInput->text(),
            phoneInput->text(), userEmailInput->text(), userBirthdayInput->text()};
}

// Проверка корректности ввода данных
bool addAddressBookItemDialog::validateInput() {
    // Регулярное выражение для проверки фамилии, имени, отчества
    QRegularExpression nameRegex("^[A-Z]+([ -]?[A-Za-z0-9]+)*$");
    if (!nameRegex.match(userLastNameInput->text().trimmed()).hasMatch() ||
        !nameRegex.match(userFirstNameInput->text().trimmed()).hasMatch() ||
        !nameRegex.match(userPatronymicNameInput->text().trimmed()).hasMatch()) {
        QMessageBox::warning(this, "Ошибка", "Фамилия, имя и отчество должны начинаться с буквы, "
                                             "могут содержать дефис, пробел и цифры, но не могут заканчиваться или начинаться на дефис.");
        return false;
    }

    // Регулярное выражение для проверки телефона
    QRegularExpression phoneRegex("^\\+7\\d{10}$");
    if (!phoneRegex.match(phoneInput->text().trimmed()).hasMatch()) {
        QMessageBox::warning(this, "Ошибка", "Телефон должен быть в формате +7XXXXXXXXXX, где X — цифры.");
        return false;
    }

    // Получение введенного e-mail из соответствующего текстового поля
    QString userEmail = userEmailInput->text().trimmed(); // Удаляем лишние пробелы

    // Регулярное выражение для проверки e-mail
    QRegularExpression emailRegex("[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!emailRegex.match(userEmail).hasMatch()) {
        QMessageBox::warning(this, "Ошибка", "E-mail должен быть в формате example@domain.com.");
        return false;
    }

    // Проверка даты рождения
    QDate userBirthday = QDate::fromString(userBirthdayInput->text().trimmed(), "dd-MM-yyyy");
    if (!userBirthday.isValid() || userBirthday >= QDate::currentDate()) {
        QMessageBox::warning(this, "Ошибка", "Дата рождения должна быть корректной и меньше текущей даты.");
        return false;
    }

    return true;
}


delAddressBookItemDialog::delAddressBookItemDialog(QWidget *parent) : QDialog(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *warningLabel = new QLabel("Вы уверены, что хотите удалить выбранный контакт?", this);
    QPushButton *deleteButton = new QPushButton("Удалить", this);
    QPushButton *cancelButton = new QPushButton("Отмена", this);

    connect(deleteButton, &QPushButton::clicked, this, &delAddressBookItemDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &delAddressBookItemDialog::reject);

    mainLayout->addWidget(warningLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}


editAddressBookItemDialog::editAddressBookItemDialog(const QStringList &item, QWidget *parent) : QDialog(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    userLastNameInput = new QLineEdit(item[0], this);
    userLastNameInput->setStyleSheet("padding: 5px; border:solid");

    userFirstNameInput = new QLineEdit(item[1], this);
    userFirstNameInput->setStyleSheet("padding: 5px; border:solid");

    userPatronymicNameInput = new QLineEdit(item[2], this);
    userPatronymicNameInput->setStyleSheet("padding: 5px; border:solid");

    phoneInput = new QLineEdit(item[3], this);
    phoneInput->setStyleSheet("padding: 5px; border:solid");

    userEmailInput = new QLineEdit(item[4], this);
    userEmailInput->setStyleSheet("padding: 5px; border:solid");

    userBirthdayInput = new QLineEdit(item[5], this);
    userBirthdayInput->setStyleSheet("padding: 5px; border:solid");

    saveButton = new QPushButton("Сохранить", this);
    cancelButton = new QPushButton("Отмена", this);

    connect(saveButton, &QPushButton::clicked, this, [this]() {
        if (validateInput()) accept();
    });

    connect(cancelButton, &QPushButton::clicked, this, &editAddressBookItemDialog::reject);

    QLabel *labelLastNameInput = new QLabel("Фамилия:", this);
    labelLastNameInput->setStyleSheet("font-weight: bold; color: #4494fc");
    mainLayout->addWidget(labelLastNameInput);
    mainLayout->addWidget(userLastNameInput);

    QLabel *labelFirstNameInput = new QLabel("Имя:", this);
    labelFirstNameInput->setStyleSheet("font-weight: bold; color: #4494fc");
    mainLayout->addWidget(labelFirstNameInput);
    mainLayout->addWidget(userFirstNameInput);

    QLabel *labelPatronymicNameInput = new QLabel("Отчество:", this);
    labelPatronymicNameInput->setStyleSheet("font-weight: bold; color: #4494fc");
    mainLayout->addWidget(labelPatronymicNameInput);
    mainLayout->addWidget(userPatronymicNameInput);

    QLabel *labelPhoneInput = new QLabel("Номер телефона:", this);
    labelPhoneInput->setStyleSheet("font-weight: bold; color: #4494fc");
    mainLayout->addWidget(labelPhoneInput);
    mainLayout->addWidget(phoneInput);

    QLabel *labelEmailInput = new QLabel("E-mail:", this);
    labelEmailInput->setStyleSheet("font-weight: bold; color: #4494fc");
    mainLayout->addWidget(labelEmailInput);
    mainLayout->addWidget(userEmailInput);

    QLabel *labelBirthdayInput = new QLabel("Дата рождения:", this);
    labelBirthdayInput->setStyleSheet("font-weight: bold; color: #4494fc");
    mainLayout->addWidget(labelBirthdayInput);
    mainLayout->addWidget(userBirthdayInput);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

QStringList editAddressBookItemDialog::getItem() const {
    return {userLastNameInput->text(), userFirstNameInput->text(), userPatronymicNameInput->text(),
            phoneInput->text(), userEmailInput->text(), userBirthdayInput->text()};
}

bool editAddressBookItemDialog::validateInput() {
    if (userLastNameInput->text().isEmpty() || userFirstNameInput->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Фамилия и имя не могут быть пустыми.");
        return false;
    }
    return true;
}
