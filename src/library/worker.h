//
// Created by vitalya on 15.12.18.
//

#ifndef DUPLICATEFILESCANNER_WORKER_H
#define DUPLICATEFILESCANNER_WORKER_H

#include "indexer.h"
#include "searcher.h"

#include <atomic>

#include <QDebug>
#include <QDir>
#include <QFileSystemWatcher>

class Worker : public QObject {
    Q_OBJECT

public:
    Worker() = default;
    Worker(QObject* parent);
    Worker(const QString& directory, QObject* parent);
    ~Worker();

    void setWorkingDirectory(const QDir& directory);

public slots:
    void Index();
    void Finish();
    void Stop();
    void SetFilesPool(const FilesPool& filesPool);
    void ChangePattern(const QString& pattern);
    void Search();

signals:
    void SetupFilesNumber(qint64 filesNumber);
    void Finished();
    void Aborted();
    void StopAll();

private:
    QDir WorkingDirectory;
    QObject* MainWindow = nullptr;
    FilesPool FilesData;
    QString Pattern;
    QFileSystemWatcher Watcher;
    std::atomic_bool NeedStop = false;
};


#endif //DUPLICATEFILESCANNER_WORKER_H
