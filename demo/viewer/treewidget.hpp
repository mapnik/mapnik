#pragma once
#include <QTreeWidget>
#include <QTreeWidgetItem>

class TreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    TreeWidget(QWidget *parent = nullptr) : QTreeWidget(parent) {
        connect(this, &TreeWidget::itemClicked, this, &TreeWidget::onItemChanged);
    }

public slots:
    void onItemChanged(QTreeWidgetItem *item, int column) {
        if(column == 0) { // 第0列是勾选框
            Qt::CheckState state = item->checkState(0);
            if(item->parent() == nullptr) { // 如果是父节点
                setChildCheckState(item, state);
            } else { // 如果是子节点
                updateParentCheckState(item->parent());
            }
        }
    }

private:
    void setChildCheckState(QTreeWidgetItem *item, Qt::CheckState state) {
        for(int i = 0; i < item->childCount(); ++i) {
            QTreeWidgetItem *child = item->child(i);
            child->setCheckState(0, state);
        }
    }

    void updateParentCheckState(QTreeWidgetItem *parent) {
        int checkedCount = 0;
        int uncheckedCount = 0;
        for(int i = 0; i < parent->childCount(); ++i) {
            switch(parent->child(i)->checkState(0)) {
                case Qt::Checked:
                    checkedCount++;
                    break;
                case Qt::Unchecked:
                    uncheckedCount++;
                    break;
                default:
                    break;
            }
        }
        if(checkedCount == parent->childCount()) {
            parent->setCheckState(0, Qt::Checked);
        } else if(uncheckedCount == parent->childCount()) {
            parent->setCheckState(0, Qt::Unchecked);
        } else {
            parent->setCheckState(0, Qt::PartiallyChecked);
        }
    }
};

