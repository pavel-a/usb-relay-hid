#include "device.h"
#include "ui_device.h"

Device::Device(qint16 index, QWidget *parent, QString name, QString serial, quint16 channelCount, QVector<bool> status) :
    QWidget(parent),
    ui(new Ui::device),
    _index(index)
{
    ui->setupUi(this);

    ui->device_channel_count_value->setText(QString::number(channelCount));
    ui->device_name_value->setText(name);
    ui->device_serial_value->setText(serial);

    connect(ui->device_close_all,&QPushButton::clicked,[this](){
        changeChannelStatus(false);
        emit closeAllSignal(this->_index);
    });

    connect(ui->device_open_all,&QPushButton::clicked,[this](){
        changeChannelStatus(true);
        emit openAllSignal(this->_index);
    });


    QVBoxLayout *vbox = new QVBoxLayout;
    for(auto i=1;i<channelCount+1;i++){
        QCheckBox *checkBox = new QCheckBox(QString("Channel %1").arg(i),this);
        connect(checkBox, &QCheckBox::clicked,[this,i](bool state){
            emit changeChannelState(this->_index, state,i);
        });
        checkBox->setChecked(status[i-1]);
        vbox->addWidget(checkBox);
    }

    ui->channels_windget->setLayout(vbox);
}

Device::~Device()
{
    delete ui;
}

void Device::changeChannelStatus(bool status)
{
    QList<QCheckBox *> allCButtons = this->ui->channels_windget->findChildren<QCheckBox *>();
    for(const auto& i:allCButtons){
        i->setChecked(status);
    }
}
