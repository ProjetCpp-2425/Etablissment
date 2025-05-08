#include "etablissement.h"
#include <QDebug>
#include <QSqlError>

Etablissement::Etablissement() {}

Etablissement::Etablissement(QString id, QString adresse, int capacite, int nbsalle, QString directeur)
{
    this->id = id;
    this->adresse = adresse;
    this->capacite = capacite;
    this->nbsalle = nbsalle;
    this->directeur = directeur;
    qDebug() << "Etablissement créé avec ID:" << id
             << ", Adresse:" << adresse << ", Capacité:" << capacite
             << ", Nombre de salles:" << nbsalle << ", Directeur:" << directeur;
}

bool Etablissement::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO etablissement (id, adresse, capacite, nbsalle, directeur) "
                  "VALUES (:id, :adresse, :capacite, :nbsalle, :directeur)");

    query.bindValue(":id", id);
    query.bindValue(":adresse", adresse);
    query.bindValue(":capacite", capacite);
    query.bindValue(":nbsalle", nbsalle);
    query.bindValue(":directeur", directeur);

    bool success = query.exec();

    if (success) {
        qDebug() << "Etablissement ajouté avec ID:" << id;
    }

    return success;
}

QSqlQueryModel* Etablissement::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;
    query.prepare("SELECT id, adresse, capacite, nbsalle, directeur FROM etablissement");

    if (!query.exec()) {
        qDebug() << "Erreur lors de l'affichage des établissements:" << query.lastError().text();
        delete model;
        return nullptr;
    }

    model->setQuery(query);
    return model;
}

bool Etablissement::modifier(QString id)
{
    QSqlQuery query;

    query.prepare("SELECT COUNT(*) FROM etablissement WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Erreur lors de la vérification de l'existence de l'établissement:" << query.lastError().text();
        return false;
    }

    query.next();


    query.prepare("UPDATE etablissement SET adresse = :adresse, capacite = :capacite, "
                  "nbsalle = :nbsalle, directeur = :directeur WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":adresse", adresse);
    query.bindValue(":capacite", capacite);
    query.bindValue(":nbsalle", nbsalle);
    query.bindValue(":directeur", directeur);

    bool success = query.exec();
    if (success) {
        qDebug() << "Établissement avec ID" << id << "modifié avec succès.";
    } else {
        qDebug() << "Échec de la modification de l'établissement avec ID" << id;
        qDebug() << "Erreur:" << query.lastError().text();
    }

    return success;
}

bool Etablissement::supprimer(QString id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM etablissement WHERE id = :id");
    query.bindValue(":id", id);

    bool success = query.exec();

    if (success) {
        if (query.numRowsAffected() > 0) {
            qDebug() << "Établissement supprimé avec succès, ID:" << id;
            return true;
        } else {
            qDebug() << "Aucun établissement trouvé avec ID:" << id;
            return false;
        }
    } else {
        qDebug() << "Échec de la suppression de l'établissement avec ID:" << id;
        qDebug() << "Erreur SQL:" << query.lastError().text();
        return false;
    }
}
