#include "completeRoadsWidget.hpp"
#include <QHeaderView>
#include <iostream>
#include <QLabel>

CompleteRoadsWidget::CompleteRoadsWidget(QWidget *parent) : QWidget(parent) {
    QHBoxLayout* hlayout1 = new QHBoxLayout();
    QHBoxLayout* hlayout2 = new QHBoxLayout();
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    QPushButton *submitButton = new QPushButton("提交选中项", this);
    m_treeWidget = new TreeWidget(this);

    //m_checkedIndexInTreeWidget = 0;
    m_nameIndexInTreeWidget = 0;
    m_idIndexInTreeWidget = 0;
    

    // 将列表控件和按钮添加到布局中
    //所属编组
    m_groupidComboBox = new QComboBox(this);
    QLabel *groupIdLabel = new QLabel("所属编组", this);
    groupIdLabel->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    groupIdLabel->setBuddy(m_groupidComboBox);
    hlayout1->addWidget(groupIdLabel);
    hlayout1->addStretch();
    hlayout1->addWidget(m_groupidComboBox);
    hlayout1->addStretch();
    vlayout->addLayout(hlayout1);

    //已有版本
    m_groupversionComboBox = new QComboBox(this);
    QLabel *groupVersionLabel = new QLabel("已有版本", this);
    groupVersionLabel->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    groupVersionLabel->setBuddy(m_groupversionComboBox);
    hlayout2->addWidget(groupVersionLabel);
    hlayout2->addStretch();
    hlayout2->addWidget(m_groupversionComboBox);
    hlayout2->addStretch();
    vlayout->addLayout(hlayout2);

    vlayout->addWidget(m_treeWidget);
    vlayout->addWidget(submitButton);

    // 连接按钮的点击信号到槽函数
    connect(submitButton, SIGNAL(clicked(bool)), this, SLOT(submitCheckedItems()));

    connect(m_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(OnItemChanged(QTreeWidgetItem*,int)));
    connect(m_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(OnItemBlinking(QTreeWidgetItem*,int)));

    // 连接 m_groupidComboBox 的信号到自定义的槽函数
    connect(m_groupidComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateVersionComboBox(int)));

    this->setLayout(vlayout);
}

void CompleteRoadsWidget::updateVersionComboBox(int index) 
{
    // 根据 m_groupidComboBox 的选项变化更新 m_groupversionComboBox 的列表信息
    if (m_groupidComboBox && m_groupversionComboBox) 
    {
        m_groupversionComboBox->clear(); // 清空 m_groupversionComboBox 的当前列表
        // 根据 m_groupidComboBox 选择的不同，为 m_groupversionComboBox 添加不同的选项
        GroupInfo data = m_groupidComboBox->itemData(index).value<GroupInfo>();
        for (int number : data.versions) 
        {
            m_groupversionComboBox->addItem(QString::number(number));
        }
    }
}

void CompleteRoadsWidget::OnItemBlinking(QTreeWidgetItem* item, int column) 
{
    if (column == m_nameIndexInTreeWidget) 
    { // 指定复选框所在的列

        //父节点不触发消息，叶子节点才触发消息
        if (item->childCount())
        {
            return;
        }
        // 获取复选框状态
        Qt::CheckState state = item->checkState(column);

        // 获取其他列的值
        QString id = item->data(m_idIndexInTreeWidget, Qt::UserRole).toString();

        // 根据复选框状态和其他列的值执行操作
        if (state == Qt::Checked)
        {
            // 复选框被选中
            emit itemBlinking_signal(id, 1);
        } else {
            // 复选框未被选中
            emit itemBlinking_signal(id, 0);
        }
    }
}

void CompleteRoadsWidget::OnItemChanged(QTreeWidgetItem* item,int column)
{
    if (column == m_nameIndexInTreeWidget) 
    { // 指定复选框所在的列

        //父节点不触发消息，叶子节点才触发消息
        if (item->childCount())
        {
            return;
        }
        // 获取复选框状态
        Qt::CheckState state = item->checkState(column);

        // 获取其他列的值
        QString id = item->data(m_idIndexInTreeWidget, Qt::UserRole).toString();

        // 根据复选框状态和其他列的值执行操作
        if (state == Qt::Checked)
        {
            // 复选框被选中
            emit itemCheckBoxChanged_signal(id, 1);
        } else {
            // 复选框未被选中
            emit itemCheckBoxChanged_signal(id, 0);
        }
    }
}

void CompleteRoadsWidget::updateGroupidComboBox(const std::vector<GroupInfo>& groupInfoList)
{
    if (m_groupidComboBox && groupInfoList.size()>0)
    {
        // 清除comboBox中的所有现有项
        m_groupidComboBox->clear();

        // 添加新的列表项
        for (auto & info : groupInfoList)
        {
            // 添加带有文本和自定义数据的项目
            QVariant var;
            var.setValue(info); // 将 MyData 实例转换为 QVariant
            m_groupidComboBox->addItem(info.name, var);
        }

        // 设置默认选中第一项
        m_groupidComboBox->setCurrentIndex(0);
    }
}

void CompleteRoadsWidget::updateOneCheckedItem(QString id, int status) 
{
    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it) {
        QTreeWidgetItem* item = *it;
        if (item->data(m_idIndexInTreeWidget, Qt::UserRole).toString() == id) 
        {
            Qt::CheckState state = status == 1 ? Qt::Checked : Qt::Unchecked;
            item->setCheckState(m_nameIndexInTreeWidget, state);
            m_treeWidget->onItemChanged(item, m_idIndexInTreeWidget);
            break;
        }
        ++it;
    }
}

void CompleteRoadsWidget::updateCheckedItems(const std::map<std::string, std::vector<cehuidataInfo>>& cehuidataInfoMap)
{
    qDebug() << "CompleteRoadsWidget::updateCheckedItems:cehuidataInfoMap size " << cehuidataInfoMap.size();
    m_treeWidget->clear();
    m_treeWidget->setColumnCount(1); // 设置列数为1
    QStringList headers;
    headers << "名称";
    m_treeWidget->setHeaderLabels(headers); // 设置标题

    // 设置列宽度自适应
    m_treeWidget->header()->setStretchLastSection(false); // 禁止最后一列自动填充剩余空间
    m_treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents); // 设置列宽度自适应内容

    for (const auto& pair : cehuidataInfoMap) {
        // 创建一个父节点
        QTreeWidgetItem* parentItem = new QTreeWidgetItem(m_treeWidget);
        parentItem->setText(m_nameIndexInTreeWidget, QString::fromStdString(pair.first)); // 设置父节点的文本为map的键
        parentItem->setFlags(parentItem->flags() | Qt::ItemIsUserCheckable); // 设置为可复选
        parentItem->setCheckState(m_nameIndexInTreeWidget, Qt::Checked); // 默认选中

        // 遍历与键关联的vector
        for (const auto& cehuidata : pair.second) {
            // 为每个cehuidataInfo对象创建一个子节点
            QTreeWidgetItem* childItem = new QTreeWidgetItem(parentItem);
            QVariant idVar;
            idVar.setValue(QString(cehuidata.ID.c_str()));
            childItem->setData(m_idIndexInTreeWidget, Qt::UserRole, idVar);
            childItem->setText(m_nameIndexInTreeWidget, cehuidata.NAME.c_str());
            childItem->setFlags(childItem->flags() | Qt::ItemIsUserCheckable); // 设置为可复选
            childItem->setCheckState(m_nameIndexInTreeWidget, Qt::Checked); // 默认选中
        }
    }
}

void CompleteRoadsWidget::submitCheckedItems()
{
    int index = m_groupidComboBox->currentIndex();
    GroupInfo data = m_groupidComboBox->itemData(index).value<GroupInfo>();
    QString id = data.id;
    QString version = m_groupversionComboBox->currentText();
    emit exportCompleteRoads_signal(data.id, version);
}
