#include "utils.h"
#include <cmath>
#include <vector>
#include <QDir>
#include <QStandardPaths>
#include "botan_all.h"

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

QString Utils::getBotanVersion()
{
    return QString::fromStdString(Botan::version_string());
}

QString Utils::getTempPath()
{
    QString configPath;

#if defined(Q_OS_WIN)
    configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif defined(Q_OS_MACOS)
    configPath = QDir::fromNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
#else
    configPath = QDir::fromNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
#endif

    configPath += "/temp/";
    return (configPath);
}

void Utils::clearDir(const QString &dir_path)
{
    QDir qd(dir_path);

    if (qd.exists()) {
        QList<QFileInfo> item_list = qd.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoSymLinks |
                                                      QDir::NoDotAndDotDot | QDir::Hidden);

        for (auto it = item_list.begin(); it != item_list.end(); ++it) {
            if (it->isDir())
                QDir(it->absoluteFilePath()).removeRecursively();
            else if (it->isFile())
                QDir().remove(it->absoluteFilePath());
        }
    }
}

qint64 Utils::getDirSize(const QString &curr_dir)
{
    qint64 size = 0;

    // get a list of elements in the current directory

    QDir qdir             = QDir(curr_dir);
    QList<QFileInfo> list = qdir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoSymLinks |
                                               QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);

    for (int i = 0; i < list.size(); i++) {
        QFileInfo file = list.at(i);

        // otherwise, continue parsing files and directories
        if (file.isFile())
            size += file.size();
        else if (file.isDir())
            size += getDirSize(file.absoluteFilePath());
    }

    return size;
}

QString Utils::getFileSize(qint64 size)
{
    static const double KiB = pow(2, 10);
    static const double MiB = pow(2, 20);
    static const double GiB = pow(2, 30);
    static const double TiB = pow(2, 40);
    static const double PiB = pow(2, 50);
    // convert to appropriate units based on the size of the item
    if (size >= 0) {
        static const int precision = 0;
        if (size < KiB) {
            return (QString::number(size, 'f', precision) + " B");
        }
        else if (size < MiB) {
            return (QString::number(size / KiB, 'f', precision) + " KiB");
        }
        else if (size < GiB) {
            return (QString::number(size / MiB, 'f', precision) + " MiB");
        }
        else if (size < TiB) {
            return (QString::number(size / GiB, 'f', precision) + " GiB");
        }
        else if (size < PiB) {
            return (QString::number(size / TiB, 'f', precision) + " TiB");
        }
        else {
            return (QString::number(size / PiB, 'f', precision) + " PiB");
        }
    }
    else {
        return ("");
    }
}
