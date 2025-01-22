#include "AddressBook.hpp"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlRecord>

AddressBook::AddressBook(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    loadAddressBook();
}

AddressBook::~AddressBook() {
    saveAddressBook();
}

void AddressBook::setupUI() {
    // Создаем таблицу
    table = new QTableWidget(this);

    // В таблице у нас будет 7 столбцов
    table->setColumnCount(7);

    // Определим названия заголовков всех шести столбцов.
    table->setHorizontalHeaderLabels(QStringList() << "#" << "ФАМИЛИЯ" << "ИМЯ" << "ОТЧЕСТВО" << "НОМЕР ТЕЛЕФОНА" << "E-MAIL" << "ДАТА РОЖДЕНИЯ");
    table->setStyleSheet("QHeaderView::section { background-color:'light grey' }");

    // А вот и бесплатная сортировка :)
    table->setSortingEnabled(true);

    // Отключаем обработчики на события редактирования (чтобы не пытались прямо в таблице редактировать)
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Установим такое поведение выбора в таблице, чтобы выделялась всегда целая строка при выборе любого из столбцов.
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Создаем макет для расположения кнопок
    QHBoxLayout *buttonLayout = new QHBoxLayout();  // Макет для кнопок

    // Создаём наши кнопочки с нужными нам стилями и добавляем их в кнопочный макет
    addButton = new QPushButton("Добавить", this);
    addButton->setStyleSheet("padding: 8px; max-width:80px; background-color:#b8c5d9; }");

    editButton = new QPushButton("Редактировать", this);
    editButton->setStyleSheet("padding: 8px; max-width:80px; background-color:#b8c5d9; }");

    deleteButton = new QPushButton("Удалить", this);
    deleteButton->setStyleSheet("padding: 8px; max-width:80px; background-color:#b8c5d9; }");

    searchButton = new QPushButton("Поиск контакта", this);
    searchButton->setStyleSheet("padding: 8px; max-width:100px; background-color:#b8c5d9; }");

    addPhoneNumberButton = new QPushButton("Добавить номер", this);
    addPhoneNumberButton->setStyleSheet("padding: 8px; max-width:100px; background-color:#b8c5d9; }");

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(searchButton);
    buttonLayout->addWidget(addPhoneNumberButton);

    // Создаём наш основной макет и запихиваем в него нашу таблицу и макет с кнопками
    QVBoxLayout *mainLayout = new QVBoxLayout();

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(table);

    // Создаем центральный виджет для QMainWindow
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    centralWidget->setFixedWidth(700);

    // Центральный виджет
    setCentralWidget(centralWidget);

    // Коннектим сигналы со слотами
    connect(addButton, &QPushButton::clicked, this, &AddressBook::addAddressBookItem);
    connect(editButton, &QPushButton::clicked, this, &AddressBook::editAddressBookItem);
    connect(deleteButton, &QPushButton::clicked, this, &AddressBook::delAddressBookItem);
    connect(searchButton, &QPushButton::clicked, this, &AddressBook::searchAddressBookItem);
    connect(addPhoneNumberButton, &QPushButton::clicked, this, &AddressBook::addPhoneNumber);
}

void AddressBook::addAddressBookItem() {
    addAddressBookItemDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList itemData = dialog.getItem();
        if (itemData.isEmpty()) return;

        Item item;
        item.userLastName = itemData[0];
        item.userFirstName = itemData[1];
        item.userPatronymicName = itemData[2];

        // Разделяем введённые номера телефонов через запятую и сохраняем как список
        QStringList userPhonesListList = itemData[3].split(",", Qt::SkipEmptyParts);
        for (QString &phone : userPhonesListList) {
            phone = phone.trimmed(); // Убираем лишние пробелы вокруг номеров
        }
        item.userPhonesList = userPhonesListList.toVector();
        item.userEmail = itemData[4];
        item.userBirthday = itemData[5];

        QString key = item.userLastName + " " + item.userFirstName + " " + item.userPatronymicName;
        items[key] = item;

        // Надо бы вынести подключение к БД в единое место и одной строкой вызывать.
        QSqlDatabase db;
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("C:/sqlite_db/address_book.db");

        if (!db.open()) {
            QMessageBox::critical(this, "Ошибка!", "Ошибка подключения к БД: " + db.lastError().text());
            return;
        }

        QSqlQuery qry;
        qry.prepare("INSERT INTO address_book (user_id, lastname, firstname, patronymic, phone_list, email, birthday) "
                    "VALUES (NULL, :lastname, :firstname, :patronymic, :phone_list, :email, :birthday)");
        //Привязка значений
        qry.bindValue(":lastname", item.userLastName);
        qry.bindValue(":firstname", item.userFirstName);
        qry.bindValue(":patronymic", item.userPatronymicName);
        qry.bindValue(":phone_list", userPhonesListList.join(",")); // В БД добавляем номера разделяя их запятыми
        qry.bindValue(":email", item.userEmail);
        qry.bindValue(":birthday", item.userBirthday);

        bool ret = qry.exec();
        if (!ret) {
            QString errText = qry.lastError().text();
            QMessageBox::critical(this, "Ошибка БД", "Ошибка добавления записи в таблицу БД: " + errText);
            return;
        }

        // Мы добавили в базу новый контакт. Вытащим его уникальный идентификатор (LAST_INSERT_ID)
        QString lastInsertUserId = qry.lastInsertId().toString();

        // Добавляем контакт в таблицу
        //Сначала мы считываем количество уже существующих строк в таблице (это количество контактов) с помощью rowCount().
        //Затем добавляем новую строку в конец таблицы с помощью insertRow(row).
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(lastInsertUserId));
        table->setItem(row, 1, new QTableWidgetItem(item.userLastName));
        table->setItem(row, 2, new QTableWidgetItem(item.userFirstName));
        table->setItem(row, 3, new QTableWidgetItem(item.userPatronymicName));
        table->setItem(row, 4, new QTableWidgetItem(userPhonesListList.join(", "))); // Отображаем все номера через запятую
        table->setItem(row, 5, new QTableWidgetItem(item.userEmail));
        table->setItem(row, 6, new QTableWidgetItem(item.userBirthday));

        db.close();

        // Теперь мы в БД добавляем запись одним запросом и полная перезапись таблицы нам не нужна
        //saveAddressBook();
    }
}


void AddressBook::editAddressBookItem() {
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите контакт для редактирования.");
        return;
    }

    // Получаем данные юзера из таблицы
    QString userId = table->item(row, 0)->text();
    QString userLastName = table->item(row, 1)->text();
    QString userFirstName = table->item(row, 2)->text();
    QString userPatronymicName = table->item(row, 3)->text();
    QStringList userPhonesList = table->item(row, 4)->text().split(", "); // Разделяем номера по запятой
    QString userEmail = table->item(row, 5)->text();
    QString userBirthday = table->item(row, 6)->text();

    // Создаем список данных для передачи в диалог редактирования
    QStringList itemData = { userLastName, userFirstName, userPatronymicName, userPhonesList.join(", "), userEmail, userBirthday };

    // Вызываем диалог редактирования
    editAddressBookItemDialog dialog(itemData, this);
    if (dialog.exec() == QDialog::Accepted) {
        // Получаем отредактированные данные
        QStringList updatedItemData = dialog.getItem();

        // Обновляем контакт в таблице
        table->setItem(row, 1, new QTableWidgetItem(updatedItemData[0])); // Фамилия
        table->setItem(row, 2, new QTableWidgetItem(updatedItemData[1])); // Имя
        table->setItem(row, 3, new QTableWidgetItem(updatedItemData[2])); // Отчество
        table->setItem(row, 4, new QTableWidgetItem(updatedItemData[3])); // Номера телефонов (объединенные через запятую)
        table->setItem(row, 5, new QTableWidgetItem(updatedItemData[4])); // E-mail
        table->setItem(row, 6, new QTableWidgetItem(updatedItemData[5])); // Дата рождения

        // Обновляем данные в карте контактов
        QString key = userLastName + " " + userFirstName + " " + userPatronymicName;
        Item &item = items[key];
        item.userLastName = updatedItemData[0];
        item.userFirstName = updatedItemData[1];
        item.userPatronymicName = updatedItemData[2];
        item.userPhonesList = updatedItemData[3].split(", ").toVector(); // Преобразуем QStringList в QVector<QString>
        item.userEmail = updatedItemData[4];
        item.userBirthday = updatedItemData[5];

        // Надо бы вынести подключение к БД в единое место и одной строкой вызывать.
        QSqlDatabase db;
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("C:/sqlite_db/address_book.db");

        if (!db.open()) {
            QMessageBox::critical(this, "Ошибка!", "Ошибка подключения к БД: " + db.lastError().text());
            return;
        }
        //ОБНОВЛЕНИЕ СТРОКИ ПО USER_ID
        QSqlQuery qry;
        qry.prepare("UPDATE address_book SET lastname=:lastname, firstname=:firstname, patronymic=:patronymic, "
                    "phone_list=:phone_list, email=:email, birthday=:birthday WHERE user_id=:user_id");
        qry.bindValue(":lastname", item.userLastName);
        qry.bindValue(":firstname", item.userFirstName);
        qry.bindValue(":patronymic", item.userPatronymicName);
        qry.bindValue(":phone_list", updatedItemData[3]); // В БД добавляем номера разделяя их запятыми
        qry.bindValue(":email", item.userEmail);
        qry.bindValue(":birthday", item.userBirthday);
        qry.bindValue(":user_id", userId);

        bool ret = qry.exec();
        if (!ret) {
            QString errText = qry.lastError().text();
            QMessageBox::critical(this, "Ошибка БД", "Ошибка добавления записи в таблицу БД: " + errText);
            return;
        }

        // Теперь мы точечно обновляем запись по уникальному user_id, и полная перезапись нам не нужна
        //saveAddressBook();
    }
}


void AddressBook::delAddressBookItem() {
    int row = table->currentRow(); // Получаем текущую выбранную строку
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите контакт для удаления.");
        return;
    }
//Из выбранной строки извлекается уникальный идентификатор пользователя для удаления.
    QString userIdForRemove = table->item(row, 0)->text();

    // Получаем ключ контакта (используем фамилию, имя и отчество для идентификации)
    QString key = table->item(row, 1)->text() + " " +
                  table->item(row, 2)->text() + " " +
                  table->item(row, 3)->text();

    // Надо бы вынести подключение куда-нибудь и подключаться к базе одной строчкой.
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/sqlite_db/address_book.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка!", "Ошибка подключения к БД: " + db.lastError().text());
        return;
    }

    QSqlQuery qry;
    qry.prepare("DELETE FROM address_book WHERE user_id=:user_id");
    qry.bindValue(":user_id", userIdForRemove);

    bool ret = qry.exec();
    if (!ret) {
        QString errText = qry.lastError().text();
        QMessageBox::critical(this, "Ошибка!", "Ошибка удаления записи из БД: " + errText);
        return;
    }
    // Удаляем контакт из словаря
    if (items.contains(key)) {
        items.remove(key);
    }

    // Удаляем строку из таблицы
    table->removeRow(row);


    // Сохраняем изменения в файл
    //saveAddressBook();

    QMessageBox::information(this, "Успех", "Контакт успешно удален!");
}




void AddressBook::searchAddressBookItem() {
    static bool searchPerformed = false; // Флаг, указывающий, был ли выполнен поиск

    if (searchPerformed) {
        // Если поиск уже выполнен, восстанавливаем видимость всех строк
        for (int i = 0; i < table->rowCount(); ++i) {
            table->setRowHidden(i, false); // Показываем все строки
        }
        searchPerformed = false; // Сбрасываем флаг
        return; // Выходим из функции
    }

    searchAddressBookItemDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString searchTerms = dialog.getSearchTerm().trimmed();
        QStringList termsList = searchTerms.split(",", Qt::SkipEmptyParts);

        bool found = false;
        for (int i = 0; i < table->rowCount(); ++i) {
            QString cellText = table->item(i, 0)->text(); // Полное имя
            QString lastName = table->item(i, 2)->text().trimmed(); // Фамилия
            QString firstName = table->item(i, 1)->text().trimmed(); // Имя
            QString patronymic = table->item(i, 3)->text().trimmed(); // Отчество
            QString phone = table->item(i, 4)->text().trimmed(); // Телефон
            QString email = table->item(i, 5)->text().trimmed(); // Email
            QString userBirthday = table->item(i, 6)->text().trimmed(); // Дата рождения

            found = false; // Сброс флага перед каждой строкой

            // Проверяем каждое введённое значение
            for (const QString &term : termsList) {
                if (cellText.contains(term.trimmed(), Qt::CaseInsensitive) ||
                    lastName.contains(term.trimmed(), Qt::CaseInsensitive) ||
                    firstName.contains(term.trimmed(), Qt::CaseInsensitive) ||
                    patronymic.contains(term.trimmed(), Qt::CaseInsensitive) ||
                    phone.contains(term.trimmed(), Qt::CaseInsensitive) ||
                    email.contains(term.trimmed(), Qt::CaseInsensitive) ||
                    userBirthday.contains(term.trimmed(), Qt::CaseInsensitive)) {
                    found = true; // Если совпадение найдено, устанавливаем флаг
                    break; // Выходим из внутреннего цикла
                }
            }
            // Логика для поиска по фамилии и имени вместе
            QStringList nameParts = searchTerms.split(" ", Qt::SkipEmptyParts);
            if (nameParts.size() >= 1) { // Проверяем, что введены хотя бы одна часть (фамилия или имя)
                // Проверяем комбинацию фамилии и имени
                if (lastName.contains(nameParts[0], Qt::CaseInsensitive)) {
                    if (nameParts.size() == 2 && firstName.contains(nameParts[1], Qt::CaseInsensitive)) {
                        found = true; // Если найдены обе части - устанавливаем флаг
                    } else if (nameParts.size() == 1) {
                        found = true; // Если только фамилия
                    }
                }
            }
            table->setRowHidden(i, !found); // Прячем строку, если совпадений нет
        }

        searchPerformed = true; // Устанавливаем флаг поиска в true
    }
}







void AddressBook::loadAddressBook() {

    /*
 * **************************************
 * Выполняем подключение к базе данных
 * **************************************
*/
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/sqlite_db/address_book.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка!", "Ошибка подключения к БД: " + db.lastError().text());
        return;
    }

    /*
     * Всегда пытаемся создать таблицу. Если она создана, то просто уведомим пользователя что таблицам найдена.
     * Конечно же в реальной программе нам не нужно будет об этом сообщать пользователю.
     */
    QSqlQuery query;
    QString queryStr = "CREATE TABLE address_book (user_id INTEGER PRIMARY KEY, lastname VARCHAR(80), "
                       "firstname VARCHAR(80), patronymic VARCHAR(80), "
                       "phone_list VARCHAR(120), email VARCHAR(80), birthday VARCHAR(30));";
    bool ret = query.exec(queryStr);
    if (!ret) {
        QString errText = query.lastError().text();
        QRegularExpression nameRegex("already exists");
        if (nameRegex.match(errText).hasMatch()) {
            QMessageBox::information(this, "Внимание!", "Таблица контактов в БД найдена.");
        } else {
            QMessageBox::critical(this, "Ошибка!", "Ошибка создания таблицы адресной книги в БД.");
            return;
        }
    }

    /*******************************************************************************************************/
    /*  Тестовый код для вставки в базу нового контакта
    После тестирования нужно будет этот код закомментировать, так как добавление будет через интерфейс */
    /*******************************************************************************************************/
    /*
    QSqlQuery qry;
    qry.prepare("INSERT INTO address_book (lastname, firstname, patronymic, phone_list, email, birthday) "
                "VALUES (:lastname, :firstname, :patronymic, :phone_list, :email, :birthday)");
    qry.bindValue(":lastname", "Vinogradova");
    qry.bindValue(":firstname", "Diana");
    qry.bindValue(":patronymic", "Nikolaevna");
    qry.bindValue(":phone_list", "+79219991999");
    qry.bindValue(":email", "diana.vinogradova@gmail.com");
    qry.bindValue(":birthday", "13-07-2003");

    ret = qry.exec();
    if (!ret) {
        QString errText = qry.lastError().text();
        QMessageBox::critical(this, "Ошибка БД", "Ошибка добавления тестой записи в таблицу БД: " + errText);
        return;
    }
    */
    /*******************************************************************************************************/


    //Выполняется запрос для извлечения данных из таблицы address_book и отображения их в интерфейсе
    if (!query.exec("SELECT * FROM address_book")) {
        QString errText = query.lastError().text();
        QMessageBox::critical(this, "Ошибка!", "Ошибка запроса к таблице в БД: " + errText);
        return;
    }
    QSqlRecord rec = query.record();
    while (query.next()) {
        Item item;

        item.userId = query.value(rec.indexOf("user_id")).toString();
        item.userLastName = query.value(rec.indexOf("lastname")).toString();
        item.userFirstName = query.value(rec.indexOf("firstname")).toString();
        item.userPatronymicName = query.value(rec.indexOf("patronymic")).toString();
        item.userEmail = query.value(rec.indexOf("email")).toString();
        item.userBirthday = query.value(rec.indexOf("birthday")).toString();

        QStringList userPhonesArrayString = query.value(rec.indexOf("phone_list")).toString().split(",", Qt::SkipEmptyParts);
        item.userPhonesList = userPhonesArrayString.toVector();

        QString key = item.userLastName + " " + item.userFirstName + " " + item.userPatronymicName;
        items[key] = item;

        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(item.userId));
        table->setItem(row, 1, new QTableWidgetItem(item.userLastName));
        table->setItem(row, 2, new QTableWidgetItem(item.userFirstName));
        table->setItem(row, 3, new QTableWidgetItem(item.userPatronymicName));
        table->setItem(row, 4, new QTableWidgetItem(userPhonesArrayString.join(", ")));
        table->setItem(row, 5, new QTableWidgetItem(item.userEmail));
        table->setItem(row, 6, new QTableWidgetItem(item.userBirthday));
    }
    db.close();
    return;

    // Тут начинается чтение из файла. Когда переключим на работу с БД, нужно будет закомментировать этот код

    QFile file("contacts.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    items.clear();
    table->setRowCount(0);

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split("|");
        if (fields.size() < 6) continue;

        Item item;
        item.userLastName = fields[0].trimmed();
        item.userFirstName = fields[1].trimmed();
        item.userPatronymicName = fields[2].trimmed();
        item.userEmail = fields[4].trimmed();
        item.userBirthday = fields[5].trimmed();

        // Преобразуем номера телефонов из строки в QVector
        QStringList userPhonesListList = fields[3].split(",", Qt::SkipEmptyParts);
        item.userPhonesList = userPhonesListList.toVector();

        QString key = item.userLastName + " " + item.userFirstName + " " + item.userPatronymicName;
        items[key] = item;

        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(item.userLastName));
        table->setItem(row, 1, new QTableWidgetItem(item.userFirstName));
        table->setItem(row, 2, new QTableWidgetItem(item.userPatronymicName));
        table->setItem(row, 3, new QTableWidgetItem(userPhonesListList.join(", "))); // Отображаем все номера
        table->setItem(row, 4, new QTableWidgetItem(item.userEmail));
        table->setItem(row, 5, new QTableWidgetItem(item.userBirthday));
    }

    file.close();
}


void AddressBook::saveAddressBook() {

    /*
     * **************************************
     * Выполняем подключение к базе данных
     * **************************************
    */
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/sqlite_db/address_book.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка!", "Ошибка подключения к БД: " + db.lastError().text());
        return;
    }

    /*
     * Понимаю, что при работе с БД мы могли бы и не удалять ВСЕ данные из таблицы, чтобы снова их добавить.
     * Гораздо правильнее было бы просто в момент изменения менять конкретные строчки таблицы, обновляя их
     * по уникальным идентификаторам в таблице, но текущий вариант сделать быстрее, так как нужно было
     * всего лишь поменять два метода, load и save.
     */

    QSqlQuery query;
    QString queryStr = "DELETE FROM address_book;";
    bool ret = query.exec(queryStr);
    if (!ret) {
        QString errText = query.lastError().text();
        QMessageBox::critical(this, "Ошибка!", "Ошибка очистики таблицы в БД: " + errText);
        return;
    }

    for (const auto &item : items) {
        // Вытащим из векторного массива все номера телефонов
        QStringList userPhonesListList = QStringList::fromVector(item.userPhonesList);

        QSqlQuery qry;
        qry.prepare("INSERT INTO address_book (user_id, lastname, firstname, patronymic, phone_list, email, birthday) "
                    "VALUES (NULL, :lastname, :firstname, :patronymic, :phone_list, :email, :birthday)");
        qry.bindValue(":lastname", item.userLastName);
        qry.bindValue(":firstname", item.userFirstName);
        qry.bindValue(":patronymic", item.userPatronymicName);
        qry.bindValue(":phone_list", userPhonesListList.join(",")); // В БД добавляем номера разделяя их запятыми
        qry.bindValue(":email", item.userEmail);
        qry.bindValue(":birthday", item.userBirthday);

        ret = qry.exec();
        if (!ret) {
            QString errText = qry.lastError().text();
            QMessageBox::critical(this, "Ошибка БД", "Ошибка добавления записи в таблицу БД: " + errText);
            return;
        }
    }
    db.close();

    return;

    // Далее начинается сохранение в файл.

    QFile file("C:/projects/course/contacts.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи.");
        return;
    }

    QTextStream out(&file);
    for (const auto &item : items) {
        out << item.userLastName << "|"
            << item.userFirstName << "|"
            << item.userPatronymicName << "|";

        // Объединяем номера телефонов через запятую
        QStringList userPhonesListList = QStringList::fromVector(item.userPhonesList);
        out << userPhonesListList.join(",") << "|";

        out << item.userEmail << "|"
            << item.userBirthday << "\n";
    }

    file.close();
}

void AddressBook::addPhoneNumber() {
    int currentRow = table->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите контакт для добавления номера.");
        return;
    }

    QString newNumber = QInputDialog::getText(this, "Добавить номер телефона", "Введите номер телефона (+7XXXXXXXXXX):").trimmed();

    // Проверка формата номера
    QRegularExpression phoneRegex("^\\+7\\d{10}$");
    if (!phoneRegex.match(newNumber).hasMatch()) {
        QMessageBox::warning(this, "Ошибка", "Телефон должен быть в формате +7XXXXXXXXXX.");
        return;
    }

    QString itemKey = table->item(currentRow, 0)->text() + " " +
                      table->item(currentRow, 1)->text() + " " +
                      table->item(currentRow, 2)->text();

    if (!items.contains(itemKey)) {
        QMessageBox::warning(this, "Ошибка", "Контакт не найден.");
        return;
    }

    Item &item = items[itemKey];

    if (item.userPhonesList.size() >= 100) {
        QMessageBox::warning(this, "Ошибка", "Нельзя добавить больше 100 номеров.");
        return;
    }

    item.userPhonesList.append(newNumber);

    // Обновляем отображение в таблице
    table->setItem(currentRow, 3, new QTableWidgetItem(QStringList::fromVector(item.userPhonesList).join(", "))); // Отображаем все номера

    saveAddressBook();

    QMessageBox::information(this, "Успех", "Номер телефона успешно добавлен!");
}


