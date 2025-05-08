#ifndef ARDUINO_H
#define ARDUINO_H

#include <QObject>
#include <QSerialPort>
#include <QString>
#include <QByteArray> // <<< AJOUTER CECI

class Arduino : public QObject
{
    Q_OBJECT

public:
    explicit Arduino(QObject *parent = nullptr);  // Constructeur
    ~Arduino();  // Destructeur

    bool openConnection(int baudRate);
    void closeConnection();  // Ferme la connexion série
    bool isConnected() const;  // Vérifie si la connexion est active
    void writeData(const QByteArray &data);  // Envoie des données au port série

signals:
    void clientFound(const QString &RFID);  // Signal émis lorsque l'UID est détecté fonction principale

private slots:
    void readData();  // Lit les données reçues du port série

private:
    QSerialPort *serialPort;  // Port série utilisé pour la communication
    bool connected;  // État de la connexion
    QByteArray m_serialBuffer; // <<< AJOUTER CE BUFFER

};

#endif // ARDUINO_H
