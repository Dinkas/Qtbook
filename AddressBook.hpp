#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QTextStream>
#include <QRegularExpression>
#include <QStringList>
#include <QFile>

#include "UI_Dialogs.h"

struct Item {
    QString userLastName;
    QString userFirstName;
    QString userPatronymicName;
    QString userEmail;
    QString userBirthday;
    QVector<QString> userPhonesList;
};


class AddressBook : public QMainWindow {
    Q_OBJECT

public:
    AddressBook(QWidget *parent = nullptr);
    ~AddressBook();

// Объявляем список слотов
private slots:
    // Слот для добавления нового айтема в книгу
    void addAddressBookItem();

    // Слот для редактирования существующего айтема
    void editAddressBookItem();

    // Слот для удаления айтема из книги
    void delAddressBookItem();

    // Слот для поиска айтема в книге
    void searchAddressBookItem();

    // Слот для загрузки данных в память (потом нужно будет решение для загрузки из файла или из базы)
    void loadAddressBook();

    // Слот для сохрнения айтемов книги (в файл или в БД)
    void saveAddressBook();

    // Добавление нового номера телефона
    void addPhoneNumber();

private:
    // Виджет нужен для табличного отображения данных
    QTableWidget *table;

    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *searchButton;
    QPushButton *loadButton;
    QPushButton *addPhoneNumberButton;

    // Можно было бы заюзать и QHash, но в QMap у нас будет сортировка.
    QMap<QString, Item> items;

    void setupUI();
};

#endif // ADDRESSBOOK_H
