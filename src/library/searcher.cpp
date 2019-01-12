//
// Created by vitalya on 11.01.19.
//

#include "searcher.h"
#include "retry.h"

#include <cassert>

#include <QDebug>

static constexpr qint32 SHIFT = 2;
static constexpr qint32 MAX_CHAR = 256;
static constexpr qint64 BUFFER_SIZE = 128 * 1024;

static inline qint32 hash(const char* str) {
    qint32 result = 0;
    for (int i = 0; i < 3; ++i) {
        result = result * MAX_CHAR + (qint32)str[i];
    }
    return result;
}

Searcher::Searcher(const QString& pattern)
    : Pattern(pattern)
{
    PatternStd = new char[pattern.size() + 1];
    memcpy(PatternStd, pattern.toLatin1().data(), pattern.size());
    PatternStd[pattern.size()] = '\0';
    assert(!Pattern.isEmpty());
    for (qint32 i = 0; i < (qint32)Pattern.size() - SHIFT; ++i) {
        Trigrams.insert(hash(PatternStd + i));
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
    memset(buffer, 'v', BUFFER_SIZE);
    delete[] buffer;
    return result;
}

bool Searcher::CheckTrigrams(const FileTrigrams& fileTrigrams) {
    return fileTrigrams.contains(Trigrams);
}

void Searcher::Process() {
    emit Started();
    for (auto it = FilesData->begin(); it != FilesData->end(); ++it) {
        if (NeedStop) {
            break;
        }
        /*if (!CheckTrigrams(it.value())) {
            continue; // file doesn't contain at least one of pattern's trigram
        }*/
        QFile file(it.key());
        emit FileProcessed();
        try {
            if (CheckFile(file)) {
                emit FileFound(file.fileName());
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