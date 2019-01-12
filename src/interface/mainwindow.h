#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "library/worker.h"

#include <memory>

#include <QApplication>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QThread>
#include <QTime>

#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QStyle>
#include <QtWidgets/QAction>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void ShowAbout();
    void SelectDirectory();
    void SearchSubstring();
    void Stop();
    void ResetThread();
    void SetupInterface();
    void PostSearchInterface(bool success);

public slots:
    void UpdateProgressBar();
    void PreIndexInterface();
    void PostIndexInterface(qint64 filesNumber);
    void PreSearchInterface();
    void PostSearchFinish();
    void PostSearchAbort();
    void AddFile(const QString& file);

signals:
    void StopAll();
    void DoSearch(const QString& pattern);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::atomic_bool NeedStop = false;
    QThread* WorkingThread = nullptr;
    QTime Time;

    QDir SelectedDirectory;
    QString BeautySelectedDirectory;
};


#endif // MAINWINDOW_H