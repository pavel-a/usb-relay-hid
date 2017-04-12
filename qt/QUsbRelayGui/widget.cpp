#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    list.searchForDevices();

    for(size_t i=0; i<list.m_devices.size(); i++){

        getRelayStatus(i);

        auto dev = list.m_devices[i];

        //Device has qobject pointer to this and will be deleated by qt system
        Device* k = new Device(i,this,dev.m_name,dev.m_serialNumber,dev.m_channelsNumber,dev.m_channelsStatus);

        connect(k,&Device::closeAllSignal,this,&Widget::onSomeonWantCloseAll);
        connect(k,&Device::openAllSignal,this,&Widget::onSomeonWantOpenAll);
        connect(k,&Device::changeChannelState,this,&Widget::onSomeoneWantToDoAnythingWithChannel);

        QListWidgetItem* j = new QListWidgetItem();
        j->setSizeHint(k->size());

        listWidgetsVector.push_back(j);

        ui->listWidget->addItem(j);
        ui->listWidget->setItemWidget(j,k);
    }
}

Widget::~Widget()
{
    delete ui;
    for(auto& i: listWidgetsVector){
        delete i;
    }
}

void Widget::getRelayStatus(int value)
{
    auto handle = usb_relay_device_open(list.m_devices[value].m_raw_pointer);
    unsigned int status;
    usb_relay_device_get_status(handle,&status);
    QVector<bool> tmp;
    tmp.resize(list.m_devices[value].m_channelsNumber);
    for (int j = 0;  j < list.m_devices[value].m_channelsNumber ;  ++j){
       tmp[j] =  0 != (status & (1 << j));
    }
    list.m_devices[value].m_channelsStatus = tmp;
    usb_relay_device_close(handle);
}

void Widget::onSomeonWantCloseAll(qint16 value)
{
    auto handle = usb_relay_device_open(list.m_devices[value].m_raw_pointer);
    usb_relay_device_close_all_relay_channel(handle);
    usb_relay_device_close(handle);
}

void Widget::onSomeonWantOpenAll(qint16 value)
{
    auto handle = usb_relay_device_open(list.m_devices[value].m_raw_pointer);
    usb_relay_device_open_all_relay_channel(handle);
    usb_relay_device_close(handle);
}

void Widget::onSomeoneWantToDoAnythingWithChannel(qint16 value, bool state, int channel)
{
    auto handle = usb_relay_device_open(list.m_devices[value].m_raw_pointer);
    if(state){
        usb_relay_device_open_one_relay_channel(handle,channel);
    }else{
        usb_relay_device_close_one_relay_channel(handle,channel);
    }
    usb_relay_device_close(handle);
}
