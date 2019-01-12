//
// Created by vitalya on 10.01.19.
//

#ifndef SUBSTRINGSEARCHER_INDEXER_H
#define SUBSTRINGSEARCHER_INDEXER_H

#include <atomic>

#include <QByteArray>
#include <QDir>
#include <QFileSystemWatcher>
#include <QHash>
#include <QSet>
#include <QFile>
#include <QObject>

using FileTrigrams = QSet<qint32>;
using FilesPool = QHash<QString, FileTrigrams>;

class Indexer : public QObject {
    Q_OBJECT

public:
    Indexer() = default;
    Indexer(const QDir& directory);
    Indexer(const QDir& directory, QFileSystemWatcher* watcher);

    void setFileSystemWatcher(QFileSystemWatcher* watcher);
    void setWorkingDirectory(const QDir& directory);

    void Process();

private:
    void CountTrigrams(QFile& file, FileTrigrams& trigrams);

signals:
    void Started();
    void Finished();
    void Found(const FilesPool&);

public slots:
    void Stop();

private:
    QFileSystemWatcher* Watcher = nullptr;
    std::atomic_bool NeedStop = false;
    QDir WorkingDirectory;
    FilesPool Data;
};


#endif //SUBSTRINGSEARCHER_INDEXER_H
