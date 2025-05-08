#include "arduino.h"
#include <QRegularExpression>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

Arduino::Arduino(QObject *parent)
    : QObject(parent), serialPort(new QSerialPort(this)), connected(false)
{
    connect(serialPort, &QSerialPort::readyRead, this, &Arduino::readData);
}

Arduino::~Arduino()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
}

#include <QSerialPortInfo>

bool Arduino::openConnection(int baudRate)
{
    if (connected) {
        qDebug() << "Déjà connecté à un port série.";
        return true;
    }

    QString forcedPortName = "COM4";  // Force the port to COM4

    qDebug() << "Tentative de connexion forcée sur le port" << forcedPortName;

    serialPort->setPortName(forcedPortName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (serialPort->open(QIODevice::ReadWrite)) {
        connected = true;
        qDebug() << "Connexion série établie sur" << forcedPortName;
        return true;
    } else {
        qDebug() << "Échec de connexion sur" << forcedPortName;
    }

    return false;
}



void Arduino::closeConnection()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        connected = false;
        qDebug() << "Connexion série fermée.";
    }
}

bool Arduino::isConnected() const
{
    return connected;
}

void Arduino::readData() {
    if (!serialPort || !serialPort->isOpen() || !serialPort->bytesAvailable()) {
        return; // Sortir si le port n'est pas prêt ou s'il n'y a rien à lire
    }

    // Ajouter les données reçues au buffer interne
    m_serialBuffer.append(serialPort->readAll());

    // Traiter le buffer ligne par ligne
    while (m_serialBuffer.contains('\n')) {
        int newlinePos = m_serialBuffer.indexOf('\n');
        if (newlinePos < 0) break; // Sécurité

        // Extraire la ligne (avec le \n)
        QByteArray line = m_serialBuffer.left(newlinePos + 1);
        // Supprimer la ligne traitée du buffer
        m_serialBuffer.remove(0, newlinePos + 1);

        // Convertir en QString et nettoyer
        QString strLine = QString::fromUtf8(line).trimmed();

        // Validation : Vérifier si la ligne est un UID attendu
        // (Exemple: 8 caractères hexadécimaux)
        QRegularExpression rfidRegex("^[0-9A-F]{8}$"); // Adapter si nécessaire

        if (!strLine.isEmpty()) {
            if (rfidRegex.match(strLine).hasMatch()) {
                // UID Valide trouvé !
                qDebug() << "UID valide isolé par Arduino::readData :" << strLine;
                emit clientFound(strLine); // Émettre le signal avec l'UID propre
            } else {
                // Donnée reçue mais non reconnue comme UID (probablement du debug Arduino)
                qDebug() << "Donnée ignorée par Arduino::readData :" << strLine;
            }
        }
        // Les lignes vides sont ignorées
    }
    // Les données partielles restent dans m_serialBuffer pour la prochaine lecture
}


void Arduino::writeData(const QByteArray &data)
{
    if (serialPort->isOpen() && isConnected()) {
        serialPort->write(data);
        // flush() n'est généralement pas nécessaire avec QSerialPort mais ne fait pas de mal
        // serialPort->flush();
    } else {
        qDebug() << "Arduino::writeData: Le port série n'est pas ouvert ou connecté.";
    }
}
