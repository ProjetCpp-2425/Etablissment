#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "connection.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    Connection c;

    qDebug() << "Attempting to establish a database connection...";
    bool test = c.createconnect();

    if(test)
    {
        w.show();
        qDebug() << "Connection successful.";
        QMessageBox::information(nullptr, QObject::tr("database is open"),
                    QObject::tr("Connection successful.\n"
                                "Click Cancel to exit."), QMessageBox::Cancel);
    }
    else
    {
        qDebug() << "Connection failed.";
        QMessageBox::critical(nullptr, QObject::tr("database is not open"),
                    QObject::tr("Connection failed.\n"
                                "Click Cancel to exit."), QMessageBox::Cancel);
    }

    return a.exec();
}
