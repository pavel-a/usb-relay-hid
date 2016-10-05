#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QAbstractListModel>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QStyledItemDelegate>
#include <QSignalMapper>
#include <QListWidgetItem>
#include "device.h"

#include <usb_relay_device.h>

/*!
 * \brief The DeviceInfo class stores anything we need to know about usb relay device
 */
class DeviceInfo{
public:
    DeviceInfo() = default;
    DeviceInfo(QString nm, QString sn, quint16 cn, usb_relay_device_info* rp, QString cun):
        m_name(nm),
        m_serialNumber(sn),
        m_channelsNumber(cn),
        m_raw_pointer(rp),
        m_customName(cun)
    {
    }

    QString m_name;
    QString m_serialNumber;
    quint16 m_channelsNumber;
    usb_relay_device_info* m_raw_pointer;
    QString m_customName;
    QVector<bool> m_channelsStatus;
};

Q_DECLARE_METATYPE(DeviceInfo)


class DeviceList
{
public:
    DeviceList() = default;

    void searchForDevices(){
        auto device = usb_relay_device_enumerate();
        if(device == nullptr){
            m_devices.clear();
            return;
        }
        bool hasNext = true;
        while(hasNext){
            m_devices.push_back(DeviceInfo(device->device_path,device->serial_number,device->type,device,""));

            if(device->next != nullptr){
                device++;
            }else{
                hasNext = false;
            }
        }
    }

    ~DeviceList() = default;

    std::vector<DeviceInfo> m_devices;
};


namespace Ui {
    class Widget;
}

/*!
 * \brief The Widget class main idea - share device id as an index in DeviceList vector
 */
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
    DeviceList list;

    QVector<QListWidgetItem*> listWidgetsVector;

    void getRelayStatus(int);

private slots:
    /*!
     * \brief onSomeonWantCloseAll
     * \param value device id - see brief of a class
     */
    void onSomeonWantCloseAll(qint16 value);
    /*!
     * \brief onSomeonWantOpenAll
     * \param value device id - see brief of a class
     */
    void onSomeonWantOpenAll(qint16 value);
    void onSomeoneWantToDoAnythingWithChannel(qint16 value,bool state,int channel);

};

#endif // WIDGET_H
