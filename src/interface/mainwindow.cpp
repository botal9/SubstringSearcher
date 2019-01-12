#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <memory>
#include <thread>
#include <QPushButton>
#include <QLineEdit>

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
        , NeedStop(false)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    QCommonStyle style;
    setWindowTitle(QString("Duplicate File FileComparator"));

    ui->actionScanDirectory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));
    ui->actionStopScanning->setIcon(style.standardIcon(QCommonStyle::SP_DialogDiscardButton));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->selectedDirectory->hide();
    ui->progressBar->reset();
    ui->progressBar->hide();
    ui->actionStopScanning->setVisible(false);

    connect(ui->substringLine, &QLineEdit::returnPressed, this, &MainWindow::SearchSubstring);
    connect(ui->findButton, &QPushButton::clicked, this, &MainWindow::SearchSubstring);
    connect(ui->actionScanDirectory, &QAction::triggered, this, &MainWindow::SelectDirectory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::ShowAbout);
    connect(ui->actionStopScanning, &QAction::triggered, this, &MainWindow::Stop);

    Time.start();
}

MainWindow::~MainWindow() {
    Stop();
    ResetThread();
}

void MainWindow::Stop() {
    emit StopAll();
}

void MainWindow::ShowAbout() {
    QMessageBox::aboutQt(this);
}

void MainWindow::SelectDirectory() {
    SelectedDirectory = QFileDialog::getExistingDirectory(
            this,
            "Select Directory for Searching",
            QString(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QFileInfo selectedDirectoryInfo = QFileInfo(SelectedDirectory.absolutePath());
    BeautySelectedDirectory = QDir::toNativeSeparators(selectedDirectoryInfo.absoluteFilePath()).append(QDir::separator());

    if (!selectedDirectoryInfo.exists()) {
        return;
    }

    NeedStop = false;
    ResetThread();
    SetupInterface();

    WorkingThread = new QThread();
    Worker* worker = new Worker(SelectedDirectory.absolutePath(), this);
    worker->moveToThread(WorkingThread);

    connect(this, SIGNAL(StopAll()), worker, SLOT(Stop()), Qt::DirectConnection);
    connect(this, SIGNAL(DoSearch(const QString&)), worker, SLOT(ChangePattern(const QString&)));
    connect(WorkingThread, SIGNAL(started()), worker, SLOT(Index()));
    connect(WorkingThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(WorkingThread, SIGNAL(finished()), WorkingThread, SLOT (deleteLater()));
    connect(worker, SIGNAL(SetupFilesNumber(qint64)), this, SLOT(PostIndexInterface(qint64)));
    connect(worker, SIGNAL(Aborted()), this, SLOT(PostSearchAbort()));
    connect(worker, SIGNAL(Finished()), this, SLOT(PostSearchFinish()));

    WorkingThread->start();
}

void MainWindow::SearchSubstring() {
    Time.restart();
    QString pattern = ui->substringLine->text();
    emit DoSearch(pattern);
}

void MainWindow::UpdateProgressBar() {
    ui->progressBar->setValue(ui->progressBar->value() + PROCESSED_FILES_UPDATE);
}

void MainWindow::SetupInterface() {
    ui->statusAction->setText("Choose directory to search.");
    ui->statusScanned->hide();
    ui->progressBar->hide();
    ui->selectedDirectory->setText("Selected directory: " + BeautySelectedDirectory);
    ui->selectedDirectory->show();
    ui->actionStopScanning->setVisible(true);
    ui->treeWidget->clear();
    ui->treeWidget->setVisible(true);
}

void MainWindow::PreIndexInterface() {
    ui->statusAction->setText("Processing directory. Please, wait.");
    ui->treeWidget->setSortingEnabled(false);
    ui->treeWidget->clear();
}

void MainWindow::PostIndexInterface(qint64 filesNumber) {
    ui->progressBar->setRange(0, filesNumber);
    ui->statusScanned->setText(QString("Files scanned: ").append(QString::number(filesNumber)));
    ui->statusScanned->show();
    ui->statusAction->setText("Input substring or Choose new directory to search.");
    ui->statusAction->show();
}

void MainWindow::PreSearchInterface() {
    ui->statusAction->setText("Searching substring in files. Please, wait.");
    ui->treeWidget->setSortingEnabled(false);
    ui->treeWidget->clear();
    ui->progressBar->setValue(0);
    ui->progressBar->show();
}

void MainWindow::PostSearchInterface(bool success) {
    ui->treeWidget->setSortingEnabled(true);
    ui->progressBar->hide();
    ui->statusAction->show();
    if (success) {
        ui->statusAction->setText("Finished.");
    } else {
        ui->statusAction->setText("Aborted.");
    }
    qDebug("Time elapsed: %d ms", Time.elapsed());
}

void MainWindow::PostSearchFinish() {
    PostSearchInterface(/*success*/true);
}

void MainWindow::PostSearchAbort() {
    PostSearchInterface(/*success*/false);
}

void MainWindow::AddFile(const QString& file) {
    QRegExp regExp(BeautySelectedDirectory);

    QTreeWidgetItem* item = new QTreeWidgetItem();
    QFileInfo fileInfo(file);
    QString beautyName = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    beautyName.remove(regExp);

    item->setText(0, beautyName);
    ui->treeWidget->addTopLevelItem(item);
}

void MainWindow::ResetThread() {
    if (WorkingThread != nullptr && !WorkingThread->isFinished()) {
        WorkingThread->quit();
        WorkingThread->wait();
    }
    delete WorkingThread;
}
