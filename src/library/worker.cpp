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
    NeedStop = false;

    Indexer indexer(WorkingDirectory, &Watcher);
    connect(&indexer, SIGNAL(Finished(const FilesPool&)), this, SLOT(SetFilesPool(const FilesPool&)));
    connect(this, SIGNAL(StopAll()), &indexer, SLOT(Stop()), Qt::DirectConnection);

    indexer.Process();
    emit SetupFilesNumber(FilesData.size());
    emit Ready();
    qDebug() << "Index ready";
}

void Worker::Search() {
    qDebug() << "Start searching for pattern";

    Searcher searcher(Pattern, &FilesData);
    connect(&searcher, SIGNAL(FoundFile(const QString&)), MainWindow, SLOT(AddFile(const QString&)));
    connect(this, SIGNAL(StopAll()), &searcher, SLOT(Stop()), Qt::DirectConnection);

    searcher.Process();
    Finish();
}

void Worker::Process() {
    Index();
    Search();
}

void Worker::SetFilesPool(const FilesPool& filesPool) {
    FilesData = filesPool;
}

void Worker::ChangePattern(const QString& pattern) {
    if (pattern.isEmpty() || pattern == Pattern) {
        return;
    }
    qDebug() << "Pattern Changed:" << Pattern << "->" << pattern;
    Pattern = pattern;
    emit PatternChanged();
    Search();
}

void Worker::Stop() {
    qDebug() << "Stop working";
    NeedStop = true;
    StopAll();
}

void Worker::Finish() {
    qDebug() << "Searcher finished";
    if (NeedStop) {
        emit Aborted();
    } else {
        emit Finished();
    }
}
