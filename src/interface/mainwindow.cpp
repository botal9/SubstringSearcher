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

    ui->statusAction->setText("Choose directory");
    ui->statusScanned->hide();
    ui->selectedDirectory->hide();

    ui->progressBar->reset();
    ui->progressBar->hide();
    ui->actionStopScanning->setVisible(false);

    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::ReSearchDirectory);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::ReSearchDirectory);
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

    Time.restart();
    if (!selectedDirectoryInfo.exists()) {
        return;
    }

    NeedStop = false;
    SetupInterface();

    WorkingThread = new QThread();
    Worker* worker = new Worker(SelectedDirectory.absolutePath(), this);
    worker->moveToThread(WorkingThread);

    connect(WorkingThread, SIGNAL(started()), worker, SLOT(Index()));
    connect(this, SIGNAL(SearchSubstring(const QString&)), worker, SLOT(ChangePattern(const QString&)));
    connect(worker, SIGNAL(PatternChanged()), this, SLOT(SetupInterface()));
    connect(this, SIGNAL(StopAll()), worker, SLOT(Stop()), Qt::DirectConnection);
    connect(WorkingThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(WorkingThread, SIGNAL(finished()), WorkingThread, SLOT (deleteLater()));
    connect(worker, SIGNAL(SetupFilesNumber(qint64)), this, SLOT(SetupProgressBar(qint64)));
    connect(worker, SIGNAL(Aborted()), this, SLOT(PostProcessAbort()));
    connect(worker, SIGNAL(Finished()), this, SLOT(PostProcessFinish()));

    WorkingThread->start();
}

void MainWindow::ReSearchDirectory() {
    QString pattern = ui->lineEdit->text();
    emit SearchSubstring(pattern);
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

void MainWindow::SetupProgressBar(qint64 filesNumber) {
    ui->progressBar->show();
    ui->progressBar->setRange(0, filesNumber);
}

void MainWindow::UpdateProgressBar(qint64 filesNumber) {
    ui->progressBar->setValue(ui->progressBar->value() + filesNumber);
}

void MainWindow::PostProcessInterface(bool success) {
    ui->progressBar->hide();
    ui->statusScanned->setText(QString("Files scanned: ").append(QString::number(ui->progressBar->value())));
    ui->statusScanned->show();
    if (success) {
        ui->statusAction->setText("Finished");
    } else {
        ui->statusAction->setText("Aborted");
    }
    ui->statusAction->show();
    //ResetThread();
    qDebug("Time elapsed: %d ms", Time.elapsed());
}

void MainWindow::PostProcessFinish() {
    PostProcessInterface(/*success*/true);
}

void MainWindow::PostProcessAbort() {
    PostProcessInterface(/*success*/false);
}

void MainWindow::SetupInterface() {
    ui->statusAction->hide();
    ui->statusScanned->hide();
    ui->selectedDirectory->setText("Selected directory: " + BeautySelectedDirectory);
    ui->selectedDirectory->show();
    ui->actionStopScanning->setVisible(true);
    ui->treeWidget->clear();
    ui->treeWidget->setVisible(true);
}

void MainWindow::ResetThread() {
    if (WorkingThread != nullptr && !WorkingThread->isFinished()) {
        WorkingThread->quit();
        WorkingThread->wait();
    }
    delete WorkingThread;
}
