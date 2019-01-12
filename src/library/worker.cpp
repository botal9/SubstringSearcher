//
// Created by vitalya on 15.12.18.
//

#include "worker.h"

Worker::Worker(const QString &directory, QObject* parent)
    : WorkingDirectory(directory)
    , MainWindow(parent)
{
}

Worker::Worker(QObject* parent)
    : MainWindow(parent)
{
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
    connect(&indexer, SIGNAL(Found(const FilesPool&)), this, SLOT(SetFilesPool(const FilesPool&)));

    indexer.Process();
    emit SetupFilesNumber(FilesData.size());
    qDebug() << "Finished indexing";
}

void Worker::Search() {
    qDebug() << "Start searching for pattern";

    Searcher searcher(Pattern, &FilesData);

    connect(this, SIGNAL(StopAll()), &searcher, SLOT(Stop()), Qt::DirectConnection);
    connect(&searcher, SIGNAL(Started()), MainWindow, SLOT(PreSearchInterface()));
    connect(&searcher, SIGNAL(FileFound(const QString&)), MainWindow, SLOT(AddFile(const QString&)));
    connect(&searcher, SIGNAL(FileProcessed()), MainWindow, SLOT(UpdateProgressBar()));

    searcher.Process();
    Finish();
    qDebug() << "Finished searching";
}

void Worker::SetFilesPool(const FilesPool& filesPool) {
    FilesData = filesPool;
    emit SetupFilesNumber(FilesData.size());
}

void Worker::ChangePattern(const QString& pattern) {
    if (pattern.isEmpty() || pattern == Pattern) {
        return;
    }
    qDebug() << "Pattern Changed:" << Pattern << "->" << pattern;
    Pattern = pattern;
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
