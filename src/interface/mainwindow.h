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
    void ReSearchDirectory();
    void Stop();
    void PostProcessInterface(bool success);
    void ResetThread();

public slots:
    void SetupInterface();
    void AddFile(const QString& file);
    void UpdateProgressBar(qint64 filesNumber);
    void SetupProgressBar(qint64 filesNumber);
    void PostProcessFinish();
    void PostProcessAbort();

signals:
    void StopAll();
    void SearchSubstring(const QString& pattern);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::atomic_bool NeedStop = false;
    QThread* WorkingThread = nullptr;
    QTime Time;

    QDir SelectedDirectory;
    QString BeautySelectedDirectory;
};


#endif // MAINWINDOW_H