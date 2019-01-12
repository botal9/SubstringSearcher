//
// Created by vitalya on 15.12.18.
//

#include "worker.h"

Worker::Worker(const QString &directory, QObject* parent)
    : WorkingDirectory(directory)
    , MainWindow(parent)
{
    connect(&Watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(UpdateFile(const QString&)));
    connect(&Watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(UpdateDirectory(const QString&)));
}

void Worker::setWorkingDirectory(const QDir& directory) {
    WorkingDirectory = directory;
}

Worker::~Worker() {
}

void Worker::Index() {
    qDebug() << "Start indexing directory";
    if (auto files = Watcher.files(); !files.isEmpty()) {
        Watcher.removePaths(files);
    }
    if (auto dirs = Watcher.directories(); !dirs.isEmpty()) {
        Watcher.removePaths(dirs);
    }
    NeedStop = false;

    Indexer indexer(WorkingDirectory, &Watcher);
    connect(this, SIGNAL(StopAll()), &indexer, SLOT(Stop()), Qt::DirectConnection);
    connect(&indexer, SIGNAL(Started()), MainWindow, SLOT(PreIndexInterface()));
    connect(&indexer, SIGNAL(Found(const FilesPool&)), this, SLOT(SetFilesData(const FilesPool&)));

    indexer.Process();
    emit SetupFilesNumber(FilesData.size());
    qDebug() << "Finished indexing";
}

void Worker::Search() {
    qDebug() << "Start searching for pattern";
    NeedStop = false;

    Searcher searcher(Pattern, &FilesData);
    connect(this, SIGNAL(StopAll()), &searcher, SLOT(Stop()), Qt::DirectConnection);
    connect(&searcher, SIGNAL(Started()), MainWindow, SLOT(PreSearchInterface()));
    connect(&searcher, SIGNAL(FileFound(const QString&)), MainWindow, SLOT(AddFile(const QString&)));
    connect(&searcher, SIGNAL(FileProcessed()), MainWindow, SLOT(UpdateProgressBar()));
    connect(&searcher, SIGNAL(RemoveFile(const QString&)), this, SLOT(RemoveFile(const QString&)));

    searcher.Process();
    Finish();
    qDebug() << "Finished searching";
}

void Worker::SetFilesData(const FilesPool& filesPool) {
    FilesData = filesPool;
}

void Worker::UpdateFilesData(const FilesPool& filesPool) {
    for (const auto& key : filesPool.keys()) {
        FilesData.remove(key);
    }
    FilesData.unite(filesPool);
}

void Worker::ChangePattern(const QString& pattern) {
    if (pattern.isEmpty()) {
        return;
    }
    if (Pattern != pattern) {
        qDebug() << "Pattern Changed:" << Pattern << "->" << pattern;
        Pattern = pattern;
    }
    Search();
}

void Worker::Stop() {
    qDebug() << "Stop working";
    NeedStop = true;
    StopAll();
}

void Worker::Finish() {
    if (NeedStop) {
        emit Aborted();
    } else {
        emit Finished();
    }
}

void Worker::RemoveFile(const QString& fileName) {
    FilesData.remove(fileName);
    Watcher.removePath(fileName);
}

void Worker::UpdateFile(const QString& fileName) {
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists() || !fileInfo.isFile() || !fileInfo.permission(QFile::ReadUser)) {
        RemoveFile(fileName);
        return;
    }

    FilesData[fileName].clear();

    Indexer indexer;
    QFile file(fileName);
    indexer.CountTrigrams(file, FilesData[fileName]);
}

void Worker::UpdateDirectory(const QString& directory) {
    QFileInfo directoryInfo(directory);
    if (!directoryInfo.exists()) {
        Watcher.removePath(directory);
        return;
    }

    QDir dir(directory);
    for (const QString& fileName : dir.entryList()) {
        Watcher.addPath(fileName);
        UpdateFile(fileName);
    }
}
