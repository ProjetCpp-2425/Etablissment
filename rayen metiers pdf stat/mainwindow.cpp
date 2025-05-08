#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QPdfWriter>
#include <QPainter>
#include <QFileDialog>
#include <QDebug>
#include <QTextDocument> // Needed for PDF generation with HTML
#include <QRegularExpression> // Needed for validation in search

// QtCharts includes
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QtCharts/QPieSlice> // Include QPieSlice

#include <QToolTip> // à inclure tout en haut
#include <QSystemTrayIcon> // Include if used, currently not used in provided snippets

#include "import.h"
#include "email.h"
#include "backup.h"

#include "arduino.h" // Assume arduino.h is correctly included
#include <QTimer>
// #include <QSerialPort> // Pas nécessaire ici si Arduino.h l'inclut ou gère tout
// #include <QSerialPortInfo>
#include <QByteArray> // Peut être nécessaire pour writeData si non inclus ailleurs

// Include Etablissement header (assuming it exists)
#include "etablissement.h" // IMPORTANT: Make sure this header exists and is correct

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    // Initialisez model ici si vous l'utilisez comme membre et le mettez à jour
    // plutot que d'en créer de nouveaux dans chaque fonction
    , model(new QSqlQueryModel(this))
    , arduino(new Arduino(this))      // Initialize arduino here
    , etablissement() // Default construct etablissement member
    // PAS BESOIN de m_serialBuffer ici
{
    ui->setupUi(this);

    // Connect signals and slots UI
    connect(ui->pushButton_afficher, &QPushButton::clicked, this, &MainWindow::refreshTableView);
    connect(ui->buttonRechercher, &QPushButton::clicked, this, &MainWindow::on_buttonRechercher_clicked);
    connect(ui->pushButton_pdf, &QPushButton::clicked, this, &MainWindow::on_pushButton_pdf_clicked);
    connect(ui->buttonTrier, &QPushButton::clicked, this, &MainWindow::on_buttonTrier_clicked); // Connect Trier button
    connect(ui->pushButton_statistiques, &QPushButton::clicked, this, &MainWindow::on_pushButton_statistiques_clicked); // Connect Stats button
    connect(ui->import_2, &QPushButton::clicked, this, &MainWindow::on_import_2_clicked); // Connect Import button
    connect(ui->backup, &QPushButton::clicked, this, &MainWindow::on_backup_clicked); // Connect Backup button

    // CRUD Buttons
    connect(ui->pushButton_ajouter, &QPushButton::clicked, this, &MainWindow::on_pushButton_ajouter_clicked);
    connect(ui->pushButton_modifier, &QPushButton::clicked, this, &MainWindow::on_pushButton_modifier_clicked);
    connect(ui->pushButton_supprimer, &QPushButton::clicked, this, &MainWindow::on_pushButton_supprimer_clicked);

    // *** CONNEXION ARDUINO EXISTANTE (CORRECTE) ***
    // Connecte le signal émis par votre classe Arduino au slot onClientFound de MainWindow
    connect(arduino, &Arduino::clientFound, this, &MainWindow::onClientFound);
    qDebug() << "Connexion du signal Arduino::clientFound au slot MainWindow::onClientFound établie.";
    // PAS BESOIN de connecter QSerialPort::readyRead ici

    // Initialize Arduino connection
    int baudRate = 9600; // Default baud rate
    if (arduino->openConnection(baudRate)) {
        qDebug() << "Connexion Arduino initialisée avec succès!";
    } else {
        qDebug() << "Échec de l'initialisation de la connexion à l'Arduino.";
         QMessageBox::critical(this, "Erreur Connexion", "Impossible d'ouvrir le port série pour l'Arduino.");
    }

    // Initial refresh of the table view
    refreshTableView();
}
MainWindow::~MainWindow()
{
    delete ui;
    // No need to delete model and arduino if they have 'this' as parent
}

// --- CRUD Operations ---

void MainWindow::on_pushButton_ajouter_clicked()
{
    // Get data from UI elements
    QString id = ui->lineEdit_id->text().trimmed(); // Get ID as QString
    QString adresse = ui->lineEdit_adresse->text().trimmed();
    QString capaciteText = ui->lineEdit_capacite->text().trimmed();
    QString nbsalleText = ui->lineEdit_nbsalle->text().trimmed();
    QString directeur = ui->lineEdit_directeur->text().trimmed();

    // Basic validation
    if (id.isEmpty() || adresse.isEmpty() || capaciteText.isEmpty() || nbsalleText.isEmpty() || directeur.isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Tous les champs doivent être remplis.");
        return;
    }

    bool capaciteValid, nbsalleValid;
    int capacite = capaciteText.toInt(&capaciteValid);
    int nbsalle = nbsalleText.toInt(&nbsalleValid);

    if (!capaciteValid || !nbsalleValid || capacite <= 0 || nbsalle <= 0) {
        QMessageBox::warning(this, "Erreur de saisie", "La capacité et le nombre de salles doivent être des nombres entiers positifs.");
        return;
    }

    // Create Etablissement object (Assuming constructor takes QString for id)
    // IMPORTANT: The Etablissement class MUST be updated to handle QString ID internally
    Etablissement etab(id, adresse, capacite, nbsalle, directeur);

    // Attempt to add the establishment
    // IMPORTANT: The etablissement.ajouter() method MUST be updated to handle QString ID
    if (etab.ajouter()) { // Assuming ajouter is now part of the 'etab' instance
        QMessageBox::information(this, "Succès", "Établissement ajouté avec succès.");
        refreshTableView(); // Refresh view after adding

        // --- Email Notification ---
        QString message = QString("Un nouvel établissement a été ajouté:\n"
                                          "Id: %1\n"
                                          "Adresse: %2\n"
                                          "Capacité: %3\n"
                                          "Nombre de Salles: %4\n"
                                          "Directeur: %5")
                                          .arg(id)           // Already QString
                                          .arg(adresse)
                                          .arg(capacite)
                                          .arg(nbsalle)      // Added nbsalle to message
                                          .arg(directeur);

        QString recipientEmail = "mouhamed10salem@gmail.com"; // Replace with recipient email
        QString subject = "Nouvel Établissement ajouté";
        QString body = message;

        Email emailSender; // Create an instance of the Email class
        int result = emailSender.sendEmail(recipientEmail, subject, body);

        if (result == 0) {
            qDebug() << "Email envoyé avec succès.";
           // QMessageBox::information(this, "Email", "L'email de notification a été envoyé."); // Optional notification
        } else {
            qDebug() << "Échec de l'envoi de l'email.";
            QMessageBox::warning(this, "Email", "L'envoi de l'email de notification a échoué.");
        }
        // --- End Email Notification ---

         // Clear input fields after successful addition
         ui->lineEdit_id->clear();
         ui->lineEdit_adresse->clear();
         ui->lineEdit_capacite->clear();
         ui->lineEdit_nbsalle->clear();
         ui->lineEdit_directeur->clear();

    }
}

void MainWindow::on_pushButton_modifier_clicked()
{
    // Get data from UI elements for modification
    QString id = ui->lineEdit_id2->text().trimmed(); // Get ID as QString
    QString adresse = ui->lineEdit_adresse2->text().trimmed();
    QString capaciteText = ui->lineEdit_capacite2->text().trimmed();
    QString nbsalleText = ui->lineEdit_nbsalle2->text().trimmed();
    QString directeur = ui->lineEdit_directeur2->text().trimmed();

    // Basic validation
    if (id.isEmpty() || adresse.isEmpty() || capaciteText.isEmpty() || nbsalleText.isEmpty() || directeur.isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Tous les champs (y compris l'ID à modifier) doivent être remplis.");
        return;
    }

    bool capaciteValid, nbsalleValid;
    int capacite = capaciteText.toInt(&capaciteValid);
    int nbsalle = nbsalleText.toInt(&nbsalleValid);

    if (!capaciteValid || !nbsalleValid || capacite <= 0 || nbsalle <= 0) {
        QMessageBox::warning(this, "Erreur de saisie", "La capacité et le nombre de salles doivent être des nombres entiers positifs.");
        return;
    }

    // Create Etablissement object with the new data
    // IMPORTANT: Assumes Etablissement constructor takes QString id
    Etablissement etab(id, adresse, capacite, nbsalle, directeur);

    // Attempt to modify the establishment
    // IMPORTANT: The etab.modifier() method MUST be updated to accept and use QString ID
    bool success = etab.modifier(id); // Pass the QString ID to modifier

    if (success) {
        QMessageBox::information(this, "Succès", "Établissement modifié avec succès.");
        refreshTableView(); // Refresh view after modification

        // Clear input fields after successful modification
        ui->lineEdit_id2->clear();
        ui->lineEdit_adresse2->clear();
        ui->lineEdit_capacite2->clear();
        ui->lineEdit_nbsalle2->clear();
        ui->lineEdit_directeur2->clear();

    } else {
        QMessageBox::critical(this, "Erreur", "La modification a échoué. Vérifiez que l'ID existe et que les données sont valides.");
    }
}

void MainWindow::on_pushButton_supprimer_clicked()
{
    // Get the ID to delete from the corresponding line edit
    QString id = ui->lineEdit_id3->text().trimmed(); // Get ID as QString

    // Basic validation
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Veuillez entrer l'ID de l'établissement à supprimer.");
        return;
    }

    // Confirmation dialog
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation", "Êtes-vous sûr de vouloir supprimer l'établissement avec l'ID : " + id + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Attempt to delete the establishment
        // IMPORTANT: The etablissement.supprimer() method MUST be updated to accept QString ID
        bool success = etablissement.supprimer(id); // Use the member 'etablissement' instance

        if (success) {
            QMessageBox::information(this, "Succès", "Établissement supprimé avec succès.");
            refreshTableView(); // Refresh view after deletion
            ui->lineEdit_id3->clear(); // Clear the ID field
        } else {
            QMessageBox::critical(this, "Erreur", "La suppression a échoué. Vérifiez que l'ID existe.");
        }
    } else {
        qDebug() << "Suppression annulée par l'utilisateur.";
    }
}

// --- TableView Operations ---

void MainWindow::refreshTableView()
{
    Etablissement etab;
    QSqlQueryModel *model = etab.afficher();

    if (!model || model->rowCount() == 0) {
        //QMessageBox::information(this, "Aucun résultat", "Pas d'établissements dans la base.");
        delete model;
        return;
    }

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Adresse");
    model->setHeaderData(2, Qt::Horizontal, "Capacité");
    model->setHeaderData(3, Qt::Horizontal, "Nb Salles");
    model->setHeaderData(4, Qt::Horizontal, "Directeur");

    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
}

// --- PDF Export ---

void MainWindow::on_pushButton_pdf_clicked()
{
    // Prepare the query to fetch all data for the PDF
    QSqlQuery query;
    query.prepare("SELECT id, adresse, capacite, nbsalle, directeur FROM etablissement ORDER BY id ASC"); // Order for consistency

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur PDF", "Erreur lors de la récupération des données pour le PDF : " + query.lastError().text());
        return;
    }

    // Ask user for save location
    QString fileName = QFileDialog::getSaveFileName(this, "Enregistrer le PDF", "", "Fichiers PDF (*.pdf)");
    if (fileName.isEmpty()) {
        return; // User cancelled
    }
    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        fileName += ".pdf"; // Ensure .pdf extension
    }

    // Configure the PDF Writer
    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4)); // Use standard QPageSize
    pdfWriter.setResolution(300); // Good resolution for printing
    pdfWriter.setPageMargins(QMarginsF(25, 25, 25, 25)); // Margins in mm (approx)

    // Prepare HTML content for the PDF
    QString htmlContent = R"(
        <html>
        <head>
            <style>
                body { font-family: Arial, sans-serif; font-size: 10pt; }
                h1 { text-align: center; color: #333; }
                table { border-collapse: collapse; width: 100%; margin-top: 20px; }
                th, td { border: 1px solid #ccc; padding: 8px; text-align: left; }
                th { background-color: #f2f2f2; font-weight: bold; }
                tr:nth-child(even) { background-color: #f9f9f9; }
            </style>
        </head>
        <body>
            <h1>Liste des Établissements</h1>
            <table>
                <thead>
                    <tr>
                        <th>ID</th>
                        <th>Adresse</th>
                        <th>Capacité</th>
                        <th>Nombre de Salles</th>
                        <th>Directeur</th>
                    </tr>
                </thead>
                <tbody>
    )";

    // Populate the table rows
    int rowCount = 0;
    while (query.next()) {
        rowCount++;
        htmlContent += "<tr>";
        htmlContent += "<td>" + query.value(0).toString() + "</td>"; // ID (already string or converted)
        htmlContent += "<td>" + query.value(1).toString() + "</td>"; // Adresse
        htmlContent += "<td>" + query.value(2).toString() + "</td>"; // Capacite
        htmlContent += "<td>" + query.value(3).toString() + "</td>"; // Nb Salles
        htmlContent += "<td>" + query.value(4).toString() + "</td>"; // Directeur
        htmlContent += "</tr>";
    }

    htmlContent += R"(
                </tbody>
            </table>
            <p style='margin-top: 15px; font-size: 8pt; text-align: right;'>Nombre total d'établissements: )" + QString::number(rowCount) + R"(</p>
        </body>
        </html>
    )";

    // Use QTextDocument to render HTML to the PDF
    QTextDocument document;
    document.setHtml(htmlContent);
    document.setPageSize(QSizeF(pdfWriter.width(), pdfWriter.height())); // Match document size to page size
    document.print(&pdfWriter); // Use print() for better layout handling

    /* Alternative using QPainter directly (less convenient for complex layout)
    QPainter painter(&pdfWriter);
    painter.setRenderHint(QPainter::Antialiasing);
    document.drawContents(&painter);
    painter.end();
    */


    QMessageBox::information(this, "Succès", "Le fichier PDF a été généré avec succès:\n" + fileName);
}

// --- Search and Sort ---

void MainWindow::on_buttonRechercher_clicked()
{
    QString input = ui->lineEdit_recherche_3->text().trimmed();
    QString modeRecherche = ui->comboBox_18->currentText().trimmed();
    qDebug() << "Mode de recherche sélectionné :" << modeRecherche;
    qDebug() << "Terme de recherche :" << input;


    QSqlQueryModel *searchModel = new QSqlQueryModel();
    QSqlQuery query;
    QString queryString;
    bool queryPrepared = false;

    if (input.isEmpty() && modeRecherche != "Rechercher_dans_une_Fourchette_de_Capacite") {
         QMessageBox::warning(this, "Erreur de saisie", "Veuillez entrer un terme de recherche.");
         refreshTableView(); // Show all data if search term is cleared
         delete searchModel;
         return;
    }


    if (modeRecherche == "Rechercher_par_ID") {
        // Validation: Ensure ID is not empty (specific format checks removed as ID is now string)
        if (input.isEmpty()) {
            QMessageBox::warning(this, "Erreur de saisie", "Veuillez entrer un ID à rechercher.");
            delete searchModel;
            return;
        }
        QString idEtablissement = input; // ID is now QString

        queryString = "SELECT id, adresse, capacite, nbsalle, directeur FROM etablissement WHERE id = :searchTerm";
        query.prepare(queryString);
        query.bindValue(":searchTerm", idEtablissement); // Bind the QString ID
        queryPrepared = true;

    } else if (modeRecherche == "Rechercher_par_Directeur") {
        queryString = "SELECT id, adresse, capacite, nbsalle, directeur FROM etablissement WHERE LOWER(directeur) LIKE LOWER(:searchTerm)"; // Case-insensitive search
        query.prepare(queryString);
        query.bindValue(":searchTerm", "%" + input + "%"); // Wildcard search
        queryPrepared = true;

    } else if (modeRecherche == "Rechercher_dans_une_Fourchette_de_Capacite") {
         // Validate input format: two numbers separated by space
        QRegularExpression regex("^\\s*\\d{1,9}\\s+\\d{1,9}\\s*$"); // Allow spaces around numbers
        if (!regex.match(input).hasMatch()) {
            QMessageBox::warning(this, "Erreur de saisie", "Pour la fourchette de capacité, veuillez entrer deux nombres positifs séparés par un espace (ex: 100 500).");
            delete searchModel;
            return;
        }

        QStringList parts = input.split(' ', Qt::SkipEmptyParts); // Split by space, ignore multiple spaces
        if (parts.size() != 2) {
             QMessageBox::warning(this, "Erreur de saisie", "Format incorrect pour la fourchette de capacité. Utilisez: min max");
             delete searchModel;
             return;
        }

        bool okMin, okMax;
        int minCapacite = parts[0].toInt(&okMin);
        int maxCapacite = parts[1].toInt(&okMax);

        if (!okMin || !okMax || minCapacite < 0 || maxCapacite < 0) {
            QMessageBox::warning(this, "Erreur de saisie", "Les capacités doivent être des nombres entiers positifs.");
            delete searchModel;
            return;
        }

        if (minCapacite > maxCapacite) {
            QMessageBox::warning(this, "Erreur de saisie", "La capacité minimale doit être inférieure ou égale à la capacité maximale.");
            delete searchModel;
            return;
        }

        queryString = "SELECT id, adresse, capacite, nbsalle, directeur FROM etablissement WHERE capacite BETWEEN :minCap AND :maxCap ORDER BY capacite ASC";
        query.prepare(queryString);
        query.bindValue(":minCap", minCapacite);
        query.bindValue(":maxCap", maxCapacite);
        queryPrepared = true;

    } else {
        QMessageBox::warning(this, "Mode inconnu", "Mode de recherche non reconnu sélectionné.");
        delete searchModel;
        return;
    }

    // Execute the query if prepared
    if (queryPrepared) {
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur SQL", "Erreur lors de l'exécution de la recherche : " + query.lastError().text());
            delete searchModel;
            return;
        }

        qDebug() << "Recherche exécutée avec succès.";
        searchModel->setQuery(std::move(query)); // Move query to model

        // Check for errors after setting the query
         if (searchModel->lastError().isValid()) {
             QMessageBox::critical(this, "Erreur Modèle", "Erreur lors de la définition du modèle de recherche : " + searchModel->lastError().text());
             delete searchModel;
             return;
         }

        if (searchModel->rowCount() == 0) {
            QMessageBox::information(this, "Aucun résultat", "Aucun établissement ne correspond à vos critères de recherche.");
            ui->tableView->setModel(nullptr); // Clear view if no results
        } else {
            // Set headers for the search results model
             searchModel->setHeaderData(0, Qt::Horizontal, "ID");
             searchModel->setHeaderData(1, Qt::Horizontal, "Adresse");
             searchModel->setHeaderData(2, Qt::Horizontal, "Capacité");
             searchModel->setHeaderData(3, Qt::Horizontal, "Nb Salles");
             searchModel->setHeaderData(4, Qt::Horizontal, "Directeur");
            ui->tableView->setModel(searchModel); // Display results
            ui->tableView->resizeColumnsToContents();
        }
    } else {
         // Should not happen if logic is correct, but as a fallback:
         delete searchModel;
    }
     // Note: The searchModel is now owned by the tableView if set successfully.
     // If not set or if there was an error, we deleted it.
     // Consider managing model lifetime more explicitly if needed (e.g., using a member variable).
}


void MainWindow::on_buttonTrier_clicked()
{
    QString modeTri = ui->comboBox_10->currentText().trimmed();
    qDebug() << "Mode de tri sélectionné : " << modeTri;

    QSqlQueryModel *sortedModel = new QSqlQueryModel();
    QSqlQuery query;
    QString queryString;

    if (modeTri == "Trier_par_Capacite") {
        queryString = "SELECT id, adresse, capacite, nbsalle, directeur FROM etablissement ORDER BY capacite ASC";
    } else if (modeTri == "Trier_par_Nb_Salles") {
        queryString = "SELECT id, adresse, capacite, nbsalle, directeur FROM etablissement ORDER BY nbsalle ASC";
    } else if (modeTri == "Trier_par_Directeur") {
        queryString = "SELECT id, adresse, capacite, nbsalle, directeur FROM etablissement ORDER BY LOWER(directeur) ASC"; // Case-insensitive sort
    } else if (modeTri == "Trier_par_ID") { // Added sorting by ID (assuming text sort is ok)
        queryString = "SELECT id, adresse, capacite, nbsalle, directeur FROM etablissement ORDER BY id ASC";
    }
     else {
        QMessageBox::warning(this, "Mode de tri inconnu", "Mode de tri non reconnu sélectionné.");
        delete sortedModel;
        return;
    }

    query.prepare(queryString);

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur SQL", "Erreur lors de l'exécution du tri : " + query.lastError().text());
        delete sortedModel;
        return;
    }

    qDebug() << "Tri exécuté avec succès.";
    sortedModel->setQuery(std::move(query)); // Move query to model

    // Check for errors after setting the query
    if (sortedModel->lastError().isValid()) {
        QMessageBox::critical(this, "Erreur Modèle", "Erreur lors de la définition du modèle de tri : " + sortedModel->lastError().text());
        delete sortedModel;
        return;
    }


    if (sortedModel->rowCount() == 0) {
        QMessageBox::information(this, "Aucun résultat", "Aucun établissement à trier (la table est vide).");
        ui->tableView->setModel(nullptr); // Clear view
    } else {
        // Set headers for the sorted model
        sortedModel->setHeaderData(0, Qt::Horizontal, "ID");
        sortedModel->setHeaderData(1, Qt::Horizontal, "Adresse");
        sortedModel->setHeaderData(2, Qt::Horizontal, "Capacité");
        sortedModel->setHeaderData(3, Qt::Horizontal, "Nombre de Salles");
        sortedModel->setHeaderData(4, Qt::Horizontal, "Directeur");

        ui->tableView->setModel(sortedModel); // Display sorted data
        ui->tableView->resizeColumnsToContents();
        qDebug() << "Données triées affichées dans le tableau.";
    }
    // Similar note about model lifetime management as in search.
}


// --- Statistics ---

void MainWindow::on_pushButton_statistiques_clicked()
{
    QSqlQuery query;
    // Retrieve data needed for the chart (ID, Adresse, Capacite)
    query.prepare("SELECT id, adresse, capacite FROM etablissement WHERE capacite > 0"); // Exclude non-positive capacity for chart
    if (!query.exec()) {
         QMessageBox::critical(this, "Erreur Statistique", "Erreur lors de la récupération des données pour les statistiques : " + query.lastError().text());
         return;
    }

    if (!query.first()) { // Check if there is any data to display
        QMessageBox::information(this, "Statistiques", "Aucune donnée d'établissement disponible pour générer les statistiques.");
        return;
    }
     query.previous(); // Go back before the first record for the loop

    QPieSeries *series = new QPieSeries();
    series->setName("Capacité par Établissement");

    // Create a chart object
    QChart *chart = new QChart();
    chart->setTitle("Répartition de la Capacité des Établissements par Adresse");
    chart->setAnimationOptions(QChart::SeriesAnimations); // Add some animation
    chart->legend()->setVisible(true); // Show legend
    chart->legend()->setAlignment(Qt::AlignBottom); // Position legend

    while (query.next()) {
        QString id = query.value(0).toString();         // Get ID as QString
        QString adresse = query.value(1).toString();
        int capacite = query.value(2).toInt();

        // Create a slice: Label = Adresse, Value = Capacite
        QPieSlice *slice = series->append(adresse + " (ID: " + id + ")", capacite); // More descriptive label
        slice->setLabelVisible(true); // Make slice label visible (percentage by default)
        // slice->setLabelPosition(QPieSlice::LabelInsideHorizontal); // Adjust label position if needed
        slice->setLabelFont(QFont("Arial", 8)); // Adjust font if needed

        // Connect hover signal to show tooltip with details
        connect(slice, &QPieSlice::hovered, this, [=](bool state) {
            if (state) {
                // Tooltip shows ID, Adresse, and Capacite
                QString tooltipText = QString("ID: %1\nAdresse: %2\nCapacité: %3")
                                        .arg(id)       // Use QString id directly
                                        .arg(adresse)
                                        .arg(capacite);
                QToolTip::showText(QCursor::pos(), tooltipText, nullptr); // Show tooltip near cursor
            } else {
                QToolTip::hideText();
            }
        });
    }

    // Add the series to the chart
    chart->addSeries(series);

    // Set up the chart view
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Create a dialog to display the chart
    QDialog *chartDialog = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout(chartDialog);
    layout->addWidget(chartView);
    chartDialog->setLayout(layout);
    chartDialog->setWindowTitle("Statistiques des Établissements");
    chartDialog->resize(800, 600); // Make dialog larger
    chartDialog->exec(); // Show the dialog modally
}

// --- Import / Export ---

void MainWindow::on_import_2_clicked() {
    Import importer;
    // Pass 'this' as parent if Import needs it for dialogs
    importer.importDatabase(this);

    // Refresh the view after import
    refreshTableView();
}

void MainWindow::on_backup_clicked() {
    Backup backup;
    // Pass 'this' as parent if Backup needs it for dialogs
    backup.exportDatabase(this);
    // No refresh needed for backup typically
}



void MainWindow::onClientFound(const QString &rfid) {
    qDebug() << "Signal clientFound reçu avec UID brut :" << rfid;

    // 1. Clean the received RFID data
    // Remove leading/trailing whitespace and potentially other non-printables
    QString cleanedRFID = rfid.trimmed();
    qDebug() << "UID nettoyé pour requête SQL :" << cleanedRFID;

    // Check if the cleaned RFID is empty; if so, inform Arduino and stop
    if (cleanedRFID.isEmpty()) {
        qDebug() << "UID nettoyé est vide, requête ignorée.";
        // Ensure 'arduino' object is accessible and writeData is the correct method
        if (arduino) { // Good practice to check if the object exists
             arduino->writeData("ERROR_EMPTY\n"); // Inform Arduino about the empty tag
        } else {
             qDebug() << "Erreur: Pointeur Arduino non initialisé.";
        }
        return; // Stop processing for this empty tag
    }

    // 2. Prepare and execute the SQL query to find the establishment by RFID
    QSqlQuery query;
    // IMPORTANT: Assumes 'RFID' column exists in 'Etablissement' table.
    query.prepare("SELECT id, adresse, capacite, nbsalle, directeur FROM Etablissement WHERE id = :rfid_placeholder");
    query.bindValue(":rfid_placeholder", cleanedRFID); // Bind the cleaned RFID

    if (query.exec()) {
        // 3. Process the query result
        if (query.next()) { // If a row was found matching the RFID...
            // 3a. Establishment found - Retrieve its data
            QString etabId = query.value(0).toString();
            QString etabAdresse = query.value(1).toString();
            int etabCapacite = query.value(2).toInt();
            int etabNbSalle = query.value(3).toInt();
            QString etabDirecteur = query.value(4).toString();

            qDebug() << "Établissement trouvé - ID:" << etabId << "Adresse:" << etabAdresse;

            // Display the found information in a message box (Optional UI feedback)
            QString etabInfo = QString("Établissement trouvé:\n"
                                       "ID : %1\n"
                                       "Adresse : %2\n"
                                       "Capacité : %3\n"
                                       "Nb Salles : %4\n"
                                       "Directeur : %5")
                                    .arg(etabId)
                                    .arg(etabAdresse)
                                    .arg(etabCapacite)
                                    .arg(etabNbSalle)
                                    .arg(etabDirecteur);
            QMessageBox::information(this, "Établissement Identifié", etabInfo);

            // 4. Send relevant information back to Arduino for LCD
            // Prepare the message, limiting length for a typical LCD (16 chars)
            QString lcdLine1 = etabAdresse.left(16);
            QString lcdLine2 = etabDirecteur.left(16);

            // Format the message: Line1,Line2\n
            QString lcdMessage = QString("%1,%2\n").arg(lcdLine1).arg(lcdLine2);
            qDebug() << "Message préparé pour Arduino :" << lcdMessage;

            // Send the message
            if (arduino) {
                arduino->writeData(lcdMessage.toUtf8()); // Send as bytes
                qDebug() << "Données envoyées à Arduino (succès supposé).";
            } else {
                qDebug() << "Erreur: Pointeur Arduino non initialisé lors de l'envoi des données.";
            }

        } else {
            // 3b. No establishment found for this RFID
            qDebug() << "Aucun établissement trouvé pour le RFID :" << cleanedRFID;
            QMessageBox::warning(this, "Établissement Inconnu", "Aucun établissement correspondant trouvé pour cet identifiant RFID.");

            // Send "UNKNOWN" status back to Arduino
            if (arduino) {
                arduino->writeData("UNKNOWN\n");
            } else {
                 qDebug() << "Erreur: Pointeur Arduino non initialisé lors de l'envoi de UNKNOWN.";
            }
        }
    } else {
        // 5. Handle SQL Error during query execution
        qDebug() << "Erreur SQL lors de la recherche par RFID : " << query.lastError().text();
        QMessageBox::critical(this, "Erreur Base de Données", "Erreur lors de la recherche de l'établissement par RFID: " + query.lastError().text());

        // Send "ERROR" status back to Arduino
        if (arduino) {
            arduino->writeData("ERROR_DB\n");
        } else {
            qDebug() << "Erreur: Pointeur Arduino non initialisé lors de l'envoi de ERROR_DB.";
        }
    }
}
