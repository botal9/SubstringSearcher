//
// Created by vitalya on 11.01.19.
//

#ifndef SUBSTRINGSEARCHER_SEARCHER_H
#define SUBSTRINGSEARCHER_SEARCHER_H

#include "indexer.h"

#include <atomic>

#include <QByteArray>
#include <QFile>
#include <QObject>
#include <QSet>
#include <QVector>

constexpr const qint32 PROCESSED_FILES_UPDATE = (1 << 8);

class Searcher : public QObject {
    Q_OBJECT

public:
    Searcher() = default;
    Searcher(const QString& pattern);
    Searcher(const QString& pattern, const FilesPool* filesPool);
    ~Searcher();

    void SetFilesData(const FilesPool* filesPool);
    void SearchInFile(QFile &file);
    bool CheckTrigrams(const TrigramsList & trigramList);
    void Process();

private:
    QString MakeSlice(const char* buffer, const char* ptr);

public slots:
    void Stop();

signals:
    void Started();
    void FileProcessed();
    void FileFound(const QString& fileName, const QString& slice);
    void Finished();
    void RemoveFile(const QString& fileName);

private:
    QString Pattern;
    char* PatternStd = nullptr;
    FileTrigrams Trigrams;
    const FilesPool* FilesData = nullptr;
    std::atomic_bool NeedStop = false;
};


#endif //SUBSTRINGSEARCHER_SEARCHER_H
