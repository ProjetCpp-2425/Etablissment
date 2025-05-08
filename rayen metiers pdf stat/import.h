#ifndef IMPORT_H
#define IMPORT_H

#include <QObject>
#include <QWidget>
#include <QString>

class Import : public QObject {
    Q_OBJECT
public:
    explicit Import(QObject *parent = nullptr);

    // Public function to import database
    void importDatabase(QWidget *parentWidget);

private:
    // Private function to parse the CSV file
    QStringList parseCSVFile(const QString &fileName);

    // Private function to insert data into the database
    bool insertIntoDatabase(const QStringList &data);
};

#endif // IMPORT_H
