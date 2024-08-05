#ifndef COMPLETEROADSWIDGET_HPP
#define COMPLETEROADSWIDGET_HPP

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QComboBox>
#include "cehuidatainfo.hpp"
#include "groupinfo.hpp"
#include "treewidget.hpp"

class CompleteRoadsWidget : public QWidget {
    Q_OBJECT
public:
    CompleteRoadsWidget(QWidget *parent = nullptr);

signals:
    void itemCheckBoxChanged_signal(const QString& id, int status);

    void exportCompleteRoads_signal(const QString& groupid, const QString& version);

    void itemBlinking_signal(const QString& id, int status);

public slots:
    void updateCheckedItems(const std::map<std::string, std::vector<cehuidataInfo>>& cehuidataInfoList);
    void OnItemChanged(QTreeWidgetItem* item,int column);
    void OnItemBlinking(QTreeWidgetItem* item,int column);
    void updateGroupidComboBox(const std::vector<GroupInfo>& groupInfoList);
    void updateVersionComboBox(int index);
    void updateOneCheckedItem(QString id, int status);

private slots:
    void submitCheckedItems();

private:
    TreeWidget* m_treeWidget;
    int m_idIndexInTreeWidget;
    int m_nameIndexInTreeWidget;
    //int m_checkedIndexInTreeWidget;

    QComboBox *m_groupidComboBox;
    QComboBox *m_groupversionComboBox;
};

#endif //COMPLETEROADSWIDGET_HPP
