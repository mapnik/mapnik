#include "completeRoadsWidget.hpp"
#include <QHeaderView>
#include <iostream>

CompleteRoadsWidget::CompleteRoadsWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    QPushButton *submitButton = new QPushButton("提交选中项", this);
    m_treeWidget = new QTreeWidget(this);

    m_idIndexInTreeWidget = 0;
    m_nameIndexInTreeWidget = 1;
    m_checkedIndexInTreeWidget = 2;

    // 将列表控件和按钮添加到布局中
    m_groupidComboBox = new QComboBox(this);
    m_groupversionComboBox = new QComboBox(this);
    layout->addWidget(m_groupidComboBox);
    layout->addWidget(m_groupversionComboBox);
    layout->addWidget(m_treeWidget);
    layout->addWidget(submitButton);

    // 连接按钮的点击信号到槽函数
    connect(submitButton, SIGNAL(clicked(bool)), this, SLOT(submitCheckedItems()));

    connect(m_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(OnItemChanged(QTreeWidgetItem*,int)));

    // 连接 m_groupidComboBox 的信号到自定义的槽函数
    connect(m_groupidComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateVersionComboBox(int)));

    this->setLayout(layout);
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

void CompleteRoadsWidget::OnItemChanged(QTreeWidgetItem* item,int column)
{
    if (column == m_checkedIndexInTreeWidget) { // 指定复选框所在的列
        // 获取复选框状态
        Qt::CheckState state = item->checkState(column);

        // 获取其他列的值
        QString id = item->text(m_idIndexInTreeWidget);

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

void CompleteRoadsWidget::updateCheckedItems(const std::vector<cehuidataInfo>& cehuidataInfoList)
{
    qDebug() << "CompleteRoadsWidget::updateCheckedItems:cehuidataInfoList size " << cehuidataInfoList.size();
    m_treeWidget->clear();
    m_treeWidget->setColumnCount(3); // 设置列数为4
    QStringList headers;
    headers << "ID" << "名称" <<"选中状态";
    m_treeWidget->setHeaderLabels(headers); // 设置标题

    // 设置列宽度自适应
    m_treeWidget->header()->setStretchLastSection(false); // 禁止最后一列自动填充剩余空间
    m_treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents); // 设置列宽度自适应内容

    for(int i=0;i<cehuidataInfoList.size();++i)
    {
        const cehuidataInfo& cehuidata = cehuidataInfoList[i];
        // 添加一行数据
        QTreeWidgetItem *item = new QTreeWidgetItem(m_treeWidget);

       std::cout<<"cehuidata.ID:"<<cehuidata.ID<<std::endl;
       std::cout<<"cehuidata.NAME:"<<cehuidata.NAME<<std::endl;

        item->setText(m_idIndexInTreeWidget, cehuidata.ID.c_str());
        item->setText(m_nameIndexInTreeWidget, cehuidata.NAME.c_str());
        item->setCheckState(m_checkedIndexInTreeWidget, Qt::Checked); // 在第三列添加未选中的复选框
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
