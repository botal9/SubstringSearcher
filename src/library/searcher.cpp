//
// Created by vitalya on 11.01.19.
//

#include "searcher.h"
#include "retry.h"

#include <cassert>

#include <QDebug>

static constexpr qint32 SHIFT = 2;
static constexpr qint64 BUFFER_SIZE = 128 * 1024;

Searcher::Searcher(const QString& pattern)
    : Pattern(pattern)
{
    PatternStd = new char[pattern.size() + 1];
    memcpy(PatternStd, pattern.toLatin1().data(), pattern.size());
    PatternStd[pattern.size()] = '\0';
    assert(!Pattern.isEmpty());
    for (qint32 i = 0; i < (qint32)Pattern.size() - SHIFT; ++i) {
        QByteArray tmp;
        for (qint32 j = i; j < i + 3; ++j) {
            tmp.push_back(Pattern[j].toLatin1());
        }
        Trigrams.insert(tmp);
    }
}

Searcher::Searcher(const QString &pattern, const FilesPool* filesPool)
    : Searcher(pattern)
{
    FilesData = filesPool;
}

Searcher::~Searcher() {
    delete[] PatternStd;
}

void Searcher::SetFilesData(const FilesPool* filesPool) {
    FilesData = filesPool;
}

bool Searcher::CheckFile(QFile &file) {
    DoWithRetryThrows(DefaultTimeOptions, TryOpenQFile, file);

    bool result = false;
    qint64 patternShift = Pattern.size() - 1;
    char* buffer = new char[BUFFER_SIZE + 1];
    buffer[BUFFER_SIZE] = '\0';

    file.read(buffer, patternShift);
    while (!file.atEnd() && !NeedStop) {
        qint64 len = file.read(buffer + patternShift, BUFFER_SIZE - patternShift);
        buffer[len] = '\0';
        char* ptr = strstr(buffer, PatternStd);
        if (ptr) {
            result = true;
            break;
        }
        if (!file.atEnd() && len > patternShift) {
            memcpy(buffer, buffer + len - patternShift, patternShift);
        }
    }

    file.close();
    delete[] buffer;
    return result;
}

bool Searcher::CheckTrigrams(const FileTrigrams& fileTrigrams) {
    return fileTrigrams.contains(Trigrams);
}

void Searcher::Process() {
    for (auto it = FilesData->begin(); it != FilesData->end(); ++it) {
        if (NeedStop) {
            break;
        }
        /*if (!CheckTrigrams(it.value())) {
            continue; // file doesn't contain at least one of pattern's trigram
        }*/
        QFile file(it.key());
        try {
            if (CheckFile(file)) {
                emit FoundFile(file.fileName());
            }
        } catch (std::exception& e) {
            qDebug() << e.what();
        }
    }
    emit Finished();
}

void Searcher::Stop() {
    NeedStop = true;
}
