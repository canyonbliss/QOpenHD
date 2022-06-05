#ifndef QOpenHDLink_H
#define QOpenHDLink_H

#include <QObject>
#include <QtQuick>

#include <nlohmann/json.hpp>


class QUdpSocket;

/**
 * @brief This is still here for legacy, I think this is what made sure that you can drag around elements on the tablet and sync their position with
 * QOpenHD running on the ground station.
 */
class QOpenHDLink: public QObject {
    Q_OBJECT

public:
    explicit QOpenHDLink(QObject *parent = nullptr);

    Q_INVOKABLE void setWidgetLocation(QString widgetName, int alignment, int xOffset, int yOffset, bool hCenter, bool vCenter);
    Q_INVOKABLE void setWidgetEnabled(QString widgetName, bool enabled);

    Q_INVOKABLE void setGroundIP(QString address);


signals:
    void widgetLocation(QString widgetName, int alignment, int xOffset, int yOffset, bool hCenter, bool vCenter);
    void widgetEnabled(QString widgetName, bool enabled);

private slots:
    void readyRead();

private:
    QString groundAddress = "192.168.2.1";

    void processCommand(QByteArray buffer);
    void processSetWidgetLocation(nlohmann::json command);
    void processSetWidgetEnabled(nlohmann::json commandData);

    QUdpSocket *linkSocket = nullptr;
};

#endif
