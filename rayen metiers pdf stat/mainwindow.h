#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "etablissement.h"
#include <QTableView>
#include <QMainWindow>
#include "arduino.h"
#include <QTcpSocket>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_ajouter_clicked();
    void on_pushButton_modifier_clicked();
    void on_pushButton_supprimer_clicked();
    void refreshTableView();
    void on_buttonRechercher_clicked();

    void on_buttonTrier_clicked();
    void on_pushButton_pdf_clicked();
    void on_pushButton_statistiques_clicked();

    void on_import_2_clicked();
    void on_backup_clicked();


private:
    Ui::MainWindow *ui;
    QTableView *tableView;
    QSqlQueryModel *model;
    Etablissement etablissement;
    Arduino *arduino;  // Déclarez un objet Arduino pour gérer la communication
   QString arduinoPortName;
   QSerialPort *serialPort;  // Declare the serial port
   QSerialPort *serial;
   QTcpSocket *socket;
   // Declare the serial port

   // Nom du port série
       void setupArduino();          // Méthode d'initialisation Arduino
       void readFromArduino();       // Lecture des données Arduino
       void handleUID(const QString &UIDC);
       void handleData(const QByteArray &data);

       void onClientFound(const QString &rfid);
};

#endif // MAINWINDOW_H
