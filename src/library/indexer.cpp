//
// Created by vitalya on 10.01.19.
//

#include "indexer.h"
#include "retry.h"

#include <QDebug>
#include <QDirIterator>

static constexpr const qint32 MAX_CHAR = 256;
static constexpr const qint64 SHIFT = 2;
static constexpr const qint64 BUFFER_SIZE = 128 * 1024;
static constexpr const qint64 MAGIC_TRIGRAMS = 20000;

static inline qint32 hash(const char* str) {
    qint32 result = 0;
    for (int i = 0; i < 3; ++i) {
        result = result * MAX_CHAR + (qint32)str[i];
    }
    return result;
}

Indexer::Indexer(const QDir& directory)
    : WorkingDirectory(directory)
{
}

Indexer::Indexer(const QDir& directory, QFileSystemWatcher* watcher)
    : WorkingDirectory(directory)
    , Watcher(watcher)
{
}

void Indexer::setWorkingDirectory(const QDir& directory) {
    WorkingDirectory = directory;
}

void Indexer::setFileSystemWatcher(QFileSystemWatcher* watcher) {
    Watcher = watcher;
    Watcher->addPath(WorkingDirectory.absolutePath());
}

void Indexer::Stop() {
    NeedStop = true;
}

void Indexer::CountTrigrams(QFile& file, FileTrigrams& trigrams) {
    DoWithRetryThrows(DefaultTimeOptions, TryOpenQFile, file);

    char* buffer = new char[BUFFER_SIZE];

    file.read(buffer, SHIFT);
    while (!file.atEnd()) {
        if (NeedStop || trigrams.size() >= MAGIC_TRIGRAMS) {
            break;
        }
        qint64 len = file.read(buffer + SHIFT, BUFFER_SIZE - SHIFT);
        for (qint64 i = 0; i < len - SHIFT; ++i) {
            trigrams.insert(hash(buffer + i)); // trigram = 3 chars
        }
        memmove(buffer, buffer + BUFFER_SIZE - SHIFT, SHIFT);
    }

    file.close();
    delete[] buffer;
}

void Indexer::Process() {
    emit Started();
    QDirIterator it(WorkingDirectory.absolutePath(), QDir::Files | QDir::NoDotAndDotDot,
                QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    while (it.hasNext()) {
        if (NeedStop) {
            break;
        }
        QFileInfo fileInfo(it.next());

        if (!fileInfo.permission(QFile::ReadUser)) {
            continue;
        }
        if (fileInfo.isDir()) {
            Watcher->addPath(fileInfo.absolutePath());
            continue;
        } else if (!fileInfo.isFile()) {
            continue;
        }

        FileTrigrams trigrams;
        QFile file(fileInfo.absoluteFilePath());
        try {
            CountTrigrams(file, trigrams);
        } catch (std::exception& e) {
            qDebug() << e.what();
        }
        if (trigrams.size() > MAGIC_TRIGRAMS) {
            // file is binary
            continue;
        }
        Watcher->addPath(fileInfo.absoluteFilePath());
        Data[fileInfo.absoluteFilePath()] = trigrams;
    }
    emit Found(Data);
}
