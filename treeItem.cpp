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

TreeItem *TreeItem::child(int number) {
    return childItems.value(number);
}

int TreeItem::childCount() const {
    return childItems.count();
}

int TreeItem::childNumber() const {
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
    qDebug() << "Want to add item of type " << type;
    qDebug() << "Adding to item of type " << itemType;
    assert(type != ROOT);
    itemTypeAssert(type, data);

    TreeItem *item = new TreeItem(data, type, this);
    childItems.append(item);
    if (type == SONG) {
        // keep track of which songs have been fetched already.
        itemData[data["absFilePath"]] = 1;
    }
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
