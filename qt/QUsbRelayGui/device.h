#ifndef DEVICE_H
#define DEVICE_H

#include <QWidget>
#include <QCheckBox>
#include <QSignalMapper>
#include <QDebug>

namespace Ui {
class device;
}

class Device : public QWidget
{
    Q_OBJECT
public:
    explicit Device(qint16 index, QWidget *parent = 0,QString name="", QString serial="", quint16 channelCount=0, QVector<bool> status = {});
    ~Device();
signals:
    /*!
     * \brief openAllSignal signaling about opening all channels in device with id passed as parameter
     */
    void openAllSignal(qint16);
    /*!
     * \brief closeAllSignal signaling about closing all channels in device with id passed as parameter
     */
    void closeAllSignal(qint16);
    /*!
     * \brief changeChannelState signaling aboud changing state of one channel in device with id passed as parameter
     * \param handle device handle
     * \param state state open\closed
     * \param channel device channel number
     */
    void changeChannelState(qint16 handle,bool state,int channel);

private:
    Ui::device *ui;
    qint16 _index;
    /*!
     * \brief changeChannelStatus heper functions - changes view for all channels at device
     * \param status
     */
    void changeChannelStatus(bool status);
};

#endif // DEVICE_H
