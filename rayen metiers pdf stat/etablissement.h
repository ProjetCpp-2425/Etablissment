#ifndef ETABLISSEMENT_H
#define ETABLISSEMENT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Etablissement {
public:
    Etablissement();
    Etablissement(QString id, QString adresse, int capacite, int nbsalle, QString directeur);

    // Getters
    QString getId() const;
    QString getAdresse() const;
    int getCapacite() const;
    int getNbsalle() const;
    QString getDirecteur() const;

    // Setters
    void setId(QString id);
    void setAdresse(const QString &adresse);
    void setCapacite(int capacite);
    void setNbsalle(int nbsalle);
    void setDirecteur(const QString &directeur);

    // Méthodes pour un établissement
    bool ajouter();
    QSqlQueryModel* afficher();
    bool modifier(QString id);
    bool supprimer(QString id);


private:
    QString id;
    QString adresse;
    int capacite;
    int nbsalle;
    QString directeur;
};

#endif // ETABLISSEMENT_H
