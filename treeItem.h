#pragma once
#include <QList>
#include <QVariant>
#include <QHash>

class TreeItem {
    public:
        enum ITEM_TYPE {ROOT, ARTIST, SONG};
        TreeItem(const QHash<QString, QString> &data, ITEM_TYPE type, TreeItem *parent = 0);
        ~TreeItem();

        TreeItem *child(int number);
        int ChildCount() const;
        int columnCount() const;
        QVariant data() const;
        bool addChild(ITEM_TYPE type, QHash<QString, QString> data);
        bool removeChild(int position);
        bool insertChild(int position, ITEM_TYPE type, QHash<QString, QString> data);
        TreeItem *parent();
        int childNumber() const;
        ITEM_TYPE getItemType() const;
        QHash<QString, QString> getItemData() const;
        QHash<QString, QString>& getItemData();
        void sortChildren();
        TreeItem *findChildNode(const QString clue) const;
        int findChildIndex(const QString clue) const;
        QList<QString> childrenData() const;
        const QList<TreeItem*> &getChildItems() const;

    private:
        void itemTypeAssert(ITEM_TYPE type, QHash<QString, QString> &data) const;

        ITEM_TYPE itemType;
        QList<TreeItem*> childItems;
        QHash<QString, QString> itemData;
        TreeItem *parentItem;
};

template <typename T>
struct PtrLess {
    bool operator()(const T* a, const T* b) const {
        return a->data().toString() < b->data().toString();
    }
};




