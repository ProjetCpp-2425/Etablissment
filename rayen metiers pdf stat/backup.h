#ifndef BACKUP_H
#define BACKUP_H

#include <QObject>
#include <QString>
#include <QWidget>

class Backup : public QObject {
    Q_OBJECT

public:
    explicit Backup(QObject *parent = nullptr);
    void exportDatabase(QWidget *parentWidget);

private:
    bool saveToFile(const QString &fileName, const QString &data);
    QString fetchDatabaseData();
};

#endif // BACKUP_H
