#include "AddressBook.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    AddressBook AddressBook;
    AddressBook.show();

    return app.exec();
}
