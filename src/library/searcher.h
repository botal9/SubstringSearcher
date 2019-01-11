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

class Searcher : public QObject {
    Q_OBJECT

public:
    Searcher() = default;
    Searcher(const QString& pattern);
    Searcher(const QString& pattern, const FilesPool* filesPool);
    ~Searcher();

    void SetFilesData(const FilesPool* filesPool);
    bool CheckFile(QFile& file);
    bool CheckTrigrams(const FileTrigrams& fileTrigrams);
    void Process();

public slots:
    void Stop();

signals:
    void FoundFile(const QString&);
    void Finished();

private:
    QString Pattern;
    char* PatternStd = nullptr;
    QSet<QByteArray> Trigrams;
    const FilesPool* FilesData = nullptr;
    std::atomic_bool NeedStop = false;
};


#endif //SUBSTRINGSEARCHER_SEARCHER_H
