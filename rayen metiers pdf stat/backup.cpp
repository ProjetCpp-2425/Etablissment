#include "backup.h"
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlRecord>

Backup::Backup(QObject *parent) : QObject(parent) {}

// Public function to export database
void Backup::exportDatabase(QWidget *parentWidget) {
    // Open save dialog
    QString fileName = QFileDialog::getSaveFileName(parentWidget, "Save Backup", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    // Fetch data
    QString data = fetchDatabaseData();
    if (data.isEmpty()) {
        QMessageBox::critical(parentWidget, "Error", "Failed to retrieve data from the database.");
        return;
    }

    // Save to file
    if (saveToFile(fileName, data)) {
        QMessageBox::information(parentWidget, "Success", "Data exported successfully.");
    } else {
        QMessageBox::warning(parentWidget, "Error", "Failed to save data to file.");
    }
}

// Private function to fetch database data
QString Backup::fetchDatabaseData() {
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid()) {
        qDebug() << "Database connection is invalid.";
        return QString();
    }
    if (!db.isOpen()) {
        qDebug() << "Database connection is not open.";
        return QString();
    }

    QString data;
    QSqlQuery query;
    query.setForwardOnly(true); // Optimize for large result sets
    if (!query.prepare("SELECT * FROM etablissement")) {
        qDebug() << "Query Preparation Error:" << query.lastError().text();
        return QString();
    }

    if (!query.exec()) {
        qDebug() << "Query Execution Error:" << query.lastError().text();
        return QString();
    }

    QSqlRecord record = query.record();
    for (int i = 0; i < record.count(); ++i) {
        data.append(record.fieldName(i));
        if (i < record.count() - 1) data.append(",");
    }
    data.append("\n");

    while (query.next()) {
        for (int i = 0; i < record.count(); ++i) {
            data.append(query.value(i).toString());
            if (i < record.count() - 1) data.append(",");
        }
        data.append("\n");
    }

    return data;
}



// Private function to save data to file
bool Backup::saveToFile(const QString &fileName, const QString &data) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "File Error: Cannot open file for writing.";
        return false;
    }

    QTextStream out(&file);
    out << data;
    file.close();
    return true;
}
