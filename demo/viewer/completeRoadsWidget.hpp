#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>

class CompleteRoadsWidget : public QWidget {
    Q_OBJECT
public:
    CompleteRoadsWidget(QWidget *parent = nullptr);

private slots:
    void submitCheckedItems();

private:
    QListWidget * m_listWidget;
};
