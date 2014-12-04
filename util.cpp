#include "util.h"
#include <QHash>
void Util::get_metaData(QString path, QHash<QString, QString>& hash) {
    // get fileInfo first
    QFileInfo fileInfo(path);
    if (fileInfo.exists()) {
        hash["absFilePath"] = fileInfo.canonicalFilePath();
        hash["fileName"] = fileInfo.fileName();

        QByteArray byteArray = path.toUtf8();
        const char* cString = byteArray.constData();
        TagLib::FileRef f(cString);
        if (f.isNull()) {
            hash.clear();
            return;
        }
        if (!f.isNull() && f.tag()) {
            TagLib::Tag *tag = f.tag();
            QString title = QString::fromStdString(tag->title().toCString(true));
            QString artist = QString::fromStdString(tag->artist().toCString(true));
            QString album = QString::fromStdString(tag->album().toCString(true));
            hash["Title"] = title.isEmpty() ? hash["fileName"] : title;
            hash["Artist"] = artist.isEmpty() ? QString("Unknown") : artist;
            hash["Album"] = album.isEmpty() ? QString("Unknown") : album;
        }
        if (!f.isNull() && f.audioProperties()) {
            TagLib::AudioProperties *properties = f.audioProperties();
            int seconds = properties->length() % 60;
            int minutes = (properties->length() - seconds)/60;
            std::stringstream ss;
            ss << minutes << ":" << std::setfill('0')<<std::setw(2)<<seconds;
            QString l = QString::fromStdString(ss.str());
            hash["Length"] = l;
        }
    }
    // otherwise leave the hash empty.
}

QString Util::convert_length_format(int l) {
    int seconds = l % 60;
    int minutes = (l-seconds)/60;
    std::stringstream ss;
    ss << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
    return QString::fromStdString(ss.str());
}
