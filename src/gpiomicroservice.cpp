#include "gpiomicroservice.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#ifndef __windows__
#include <unistd.h>
#endif

#include <QtNetwork>
#include <QThread>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QFuture>

#include <openhd/mavlink.h>

#include "util.h"
#include "constants.h"

#include "openhd.h"
#include "powermicroservice.h"

#include "localmessage.h"


GPIOMicroservice::GPIOMicroservice(QObject *parent, MicroserviceTarget target, MavlinkType mavlink_type): MavlinkBase(parent, mavlink_type), m_target(target) {
    qDebug() << "GPIOMicroservice::GPIOMicroservice()";

    targetCompID = MAV_COMP_ID_USER2;
    localPort = 14551;

    switch (m_target) {
        case MicroserviceTargetNone:
        targetSysID = 0;
        break;
        case MicroserviceTargetAir:
        targetSysID = 253;
        connect(OpenHD::instance(), &OpenHD::save_air_gpio, this, &GPIOMicroservice::onSaveGPIO);
        break;
        case MicroserviceTargetGround:
        targetSysID = 254;
        connect(OpenHD::instance(), &OpenHD::save_ground_gpio, this, &GPIOMicroservice::onSaveGPIO);
        break;
    }

    connect(this, &GPIOMicroservice::setup, this, &GPIOMicroservice::onSetup);
}

void GPIOMicroservice::onSetup() {
    qDebug() << "GPIOMicroservice::onSetup()";

    connect(this, &GPIOMicroservice::processMavlinkMessage, this, &GPIOMicroservice::onProcessMavlinkMessage);
}


void GPIOMicroservice::onSaveGPIO(QList<int> gpio) {
    uint8_t pins = 0;
    for (int i = 0; i < 8; i++) {
        pins |= (gpio[i] & 1) << i;
    }

    mavlink_message_t msg;
    mavlink_msg_command_long_pack(255, MAV_COMP_ID_MISSIONPLANNER, &msg, targetSysID, targetCompID, OPENHD_CMD_SET_GPIOS, 0, pins, 0, 0, 0, 0, 0, 0);

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(buffer, &msg);

    sendData((char*)buffer, len);
}


void GPIOMicroservice::onProcessMavlinkMessage(mavlink_message_t msg) {
    if (msg.compid != targetCompID || msg.sysid != targetSysID) {
        return;
    }
    switch (msg.msgid) {
        case MAVLINK_MSG_ID_HEARTBEAT: {
            mavlink_heartbeat_t heartbeat;
            mavlink_msg_heartbeat_decode(&msg, &heartbeat);
            break;
        }
        case MAVLINK_MSG_ID_OPENHD_GPIO_STATE: {
            mavlink_openhd_gpio_state_t gpio_state;
            mavlink_msg_openhd_gpio_state_decode(&msg, &gpio_state);
            uint8_t pins = gpio_state.pins;

            auto gpio5 = pins >> 0 & 1;
            auto gpio6 = pins >> 1 & 1;
            auto gpio12 = pins >> 2 & 1;
            auto gpio13 = pins >> 3 & 1;
            auto gpio16 = pins >> 4 & 1;
            auto gpio19 = pins >> 5 & 1;
            auto gpio26 = pins >> 6 & 1;
            auto gpio32 = pins >> 7 & 1;

            switch (m_target) {
                case MicroserviceTargetNone:
                break;
                case MicroserviceTargetAir:
                OpenHD::instance()->set_air_gpio({gpio5, gpio6, gpio12, gpio13, gpio16, gpio19, gpio26, gpio32});
                break;
                case MicroserviceTargetGround:
                OpenHD::instance()->set_ground_gpio({gpio5, gpio6, gpio12, gpio13, gpio16, gpio19, gpio26, gpio32});
                break;
            }
            break;
        }
        default: {
            printf("GPIOMicroservice received unmatched message with ID %d, sequence: %d from component %d of system %d\n", msg.msgid, msg.seq, msg.compid, msg.sysid);
            break;
        }
    }
}

