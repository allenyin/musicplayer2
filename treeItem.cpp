#include "treeItem.h"
#include <QStringList>
#include <assert.h>
#include <QDebug>

TreeItem::TreeItem(const QHash<QString, QString> &data, ITEM_TYPE type, TreeItem *parent) {
    switch(type) {
        case ARTIST:
            assert(data.contains("Artist") && parent->getItemType() == ROOT);
            break;
        case SONG:
            assert(data.contains("Title") && data.contains("absFilePath") && parent->getItemType() == ARTIST);
            break;
        case ROOT:
            // no data for root
            assert(parent == NULL);
            break;
        default:
            qDebug() << "Unknown itemType!";
            break;
    }
    parentItem = parent;
    itemData = data;
    itemType = type;
}

TreeItem::~TreeItem() {
    qDeleteAll(childItems);
    childItems.clear();
}

TreeItem::ITEM_TYPE TreeItem::getItemType() const {
    return itemType;
}

QHash<QString, QString> TreeItem::getItemData() const {
    return itemData;
}

QHash<QString, QString>& TreeItem::getItemData() {
    return itemData;
}

void TreeItem::sortChildren() {
    // sort the childItems pointers alphabetically.
    if (itemType == ROOT || itemType == ARTIST) {
        qSort(childItems.begin(), childItems.end(), PtrLess<TreeItem>());
    }
}

TreeItem *TreeItem::findChildNode(const QString clue) const {
    if (itemType == ROOT) {
        TreeItem* child;
        foreach(child, childItems) {
            if (child->itemData["Artist"] == clue) {
                return child;
            }
        }
    }
    if (itemType == ARTIST) {
        TreeItem* child;
        foreach(child, childItems) {
            if (child->itemData["absFilePath"] == clue) {
                return child;
            }
        }
    }
    return NULL;
}

int TreeItem::findChildIndex(const QString clue) const {
    // returns where a child with a given clue is in this node's childList
    QString clueType;
    switch(itemType) {
        case ROOT:
            clueType = "Artist";
            break;
        case ARTIST:
            clueType = "absFilePath";
            break;
        default:
            return -1;
    }
    for (int i=0; i < childItems.size(); i++) {
        if ((childItems[i]->getItemData())[clueType] == clue) {
            return i;
        }
    }
    return -1;
}

TreeItem *TreeItem::child(int number) {
    return childItems.value(number);
}

int TreeItem::ChildCount() const {
    return childItems.count();
}

int TreeItem::childNumber() const {
    // returns where this node is this in its parent's childList
    if (parentItem) {
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
    }

    return 0;
}


int TreeItem::columnCount() const {
    //return itemData.count();
    return 1;   // always just 1 column
}

QVariant TreeItem::data() const {
    //return itemData.value(column);
    switch(itemType) {
        case ARTIST:
            return itemData["Artist"];
        case SONG:
            return itemData["Title"];
        default:
            return QVariant();
    }
}

bool TreeItem::addChild(ITEM_TYPE type, QHash<QString, QString> data) {
    //qDebug() << "Want to add item of type " << type;
    //qDebug() << "Adding to item of type " << itemType;
    assert(type != ROOT);
    itemTypeAssert(type, data);

    TreeItem *item = new TreeItem(data, type, this);
    childItems.append(item);
    /*
    if (type == SONG) {
        // keep track of which songs have been fetched already.
        itemData[data["absFilePath"]] = 1;
    }*/
    return true;
}

TreeItem *TreeItem::parent() {
    return parentItem;
}

bool TreeItem::removeChild(int position) {
    if (position < 0 || position > childItems.size()) {
        return false;
    }

    delete childItems.takeAt(position);
    return true;
}

bool TreeItem::insertChild(int position, ITEM_TYPE type, QHash<QString, QString> data) {
    assert(type != ROOT);
    itemTypeAssert(type, data);
    TreeItem *item = new TreeItem(data, type, this);
    childItems.insert(position, item);
    return true;
}

QList<QString> TreeItem::childrenData() const {
    if (ChildCount()==0) {
        return QList<QString>();
    }
    QList<QString> dataList;
    TreeItem *item;
    foreach(item, childItems) {
        dataList.append(item->data().toString());
    }
    return dataList;
}

void TreeItem::itemTypeAssert(ITEM_TYPE type, QHash<QString, QString> &data) const {
    switch (type) {
        case ARTIST:
            assert(data.contains("Artist") && (itemType == ROOT));
            return;
        case SONG:
            assert(data.contains("absFilePath") && data.contains("Title") && (itemType == ARTIST));
            return;
        default:
            return;
    }
}
