#pragma once
#include <sstream>
#include <iomanip>
#include <QFileInfo>
#include <QUrl>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

class Util {
public:
    // Given a path and a QHash ref, store the meta info of that file within
    // the hash
    void get_metaData(QString path, QHash<QString, QString>& hash);

    // convert from song length in seconds to min:sec format QString
    QString convert_length_format(int l);

    };
