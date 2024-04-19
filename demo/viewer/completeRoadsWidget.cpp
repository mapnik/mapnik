#include "completeRoadsWidget.hpp"


CompleteRoadsWidget::CompleteRoadsWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_listWidget = new QListWidget(this);
    QPushButton *submitButton = new QPushButton("提交选中项", this);

    // 添加带有复选框的项目
    for (int i = 0; i < 10; ++i) {
        QListWidgetItem *item = new QListWidgetItem(m_listWidget);
        item->setText(QString("数据项 %1").arg(i + 1));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }

    // 将列表控件和按钮添加到布局中
    layout->addWidget(m_listWidget);
    layout->addWidget(submitButton);

    // 连接按钮的点击信号到槽函数
    connect(submitButton, &QPushButton::clicked, this, &CompleteRoadsWidget::submitCheckedItems);

    this->setLayout(layout);
}

void CompleteRoadsWidget::submitCheckedItems() {
    QStringList checkedItems;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            checkedItems.append(item->text());
        }
    }
    qDebug() << "选中的数据项:" << checkedItems.join(", ");
}
