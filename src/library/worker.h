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
    Worker(const QString& directory, QObject* parent);
    ~Worker();

    void setWorkingDirectory(const QDir& directory);

public slots:
    void Index();
    void Finish();
    void Stop();
    void SetFilesData(const FilesPool &filesPool);
    void UpdateFilesData(const FilesPool& filesPool);
    void ChangePattern(const QString& pattern);
    void Search();

    //void UpdateFile(const QString& fileName);
    //void UpdateDirectory(const QString& directory);
    void RemoveFile(const QString& fileName);

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
