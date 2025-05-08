#include "connection.h"

Connection::Connection()
{

}

bool Connection::createconnect()
{
bool test=false;
QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
db.setDatabaseName("projetcpp");
db.setUserName("rayen");
db.setPassword("chocolat");

if (db.open())
test=true;
    return  test;
}
void Connection::closeConnect (){db.close();}
