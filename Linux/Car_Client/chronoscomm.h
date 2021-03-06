/*
    Copyright 2018 Benjamin Vedder	benjamin@vedder.se

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CHRONOSCOMM_H
#define CHRONOSCOMM_H

#include <QObject>
#include <QElapsedTimer>
#include <QUdpSocket>
#include <QTimer>
#include <vbytearrayle.h>
#include <tcpserversimple.h>
#include "gpio.h"

#define PROTOCOL_VERSION 2

typedef enum {
    COMM_MODE_UNDEFINED = 0,
    COMM_MODE_OBJECT,
    COMM_MODE_SUPERVISOR,
    COMM_MODE_SERVER
} COMM_MODE;

typedef enum {
    OPRO_MODE_STATIC = 1,
    OPRO_MODE_PREDEFINED_TRAJ,
    OPRO_MODE_VOP,
    OPRO_MODE_DTM,
    OPRO_MODE_VOP_AND_DTM,
    OPRO_MODE_AUTONOMOUS_VUT
} OPRO_OPERATION_MODE;

typedef enum {
    OPRO_TYPE_CAR = 1,
    OPRO_TYPE_BICYCLE = 2,
    OPRO_TYPE_PEDESTRIAN = 30
} OPRO_OBJECT_TYPE;

typedef enum {
    ISO_UNIT_TYPE_STEERING_DEGREES = 0,
    ISO_UNIT_TYPE_STEERING_PERCENTAGE = 1,
    ISO_UNIT_TYPE_SPEED_METER_SECOND = 2,
    ISO_UNIT_TYPE_SPEED_PERCENTAGE = 3
} ISO_UNIT_TYPE;

/*! HEAB control center statuses */
typedef enum {
	CONTROL_CENTER_STATUS_INIT = 0x00,			//!<
	CONTROL_CENTER_STATUS_READY = 0x01,			//!<
	CONTROL_CENTER_STATUS_ABORT = 0x02,			//!<
	CONTROL_CENTER_STATUS_RUNNING = 0x03,		//!<
	CONTROL_CENTER_STATUS_TEST_DONE = 0x04,		//!<
	CONTROL_CENTER_STATUS_NORMAL_STOP = 0x05	//!<
} ControlCenterStatusType;

typedef enum {
    OBJECT_NOT_READY_TO_ARM,
    OBJECT_READY_TO_ARM,
    OBJECT_READY_TO_ARM_UNAVAILABLE
} ISO_OBJECT_ARM_READINESS;

typedef struct {
    uint32_t tRel;
    double x;
    double y;
    double z;
    double heading;
    double long_speed;
    double lat_speed;
    double long_accel;
    double lat_accel;
    double curvature;
} chronos_traj_pt;

typedef struct {
    uint16_t traj_id;
    QString traj_name;
    uint16_t traj_ver;
    QVector<chronos_traj_pt> traj_pts;
    uint32_t object_id;
} chronos_traj;

typedef struct {
    quint32 transmitterID;
    double lat;
    double lon;
    double alt;
    double heading;
    quint32 gps_ms_of_week;
    quint16 gps_week;
    quint16 maxWayDeviation;
    quint16 maxLateralDeviation;
    quint16 minPosAccuracy;
} chronos_osem;

typedef struct {
    uint8_t objectType;
    uint8_t actorType;
    uint8_t operationMode;
    uint32_t mass;
    uint32_t objectLengthX;
    uint32_t objectLengthY;
    uint32_t objectLengthZ;
    int16_t positionDisplacementX;
    int16_t positionDisplacementY;
    int16_t positionDisplacementZ;
} chronos_opro;

typedef struct {
    int state;
} chronos_ostm;

typedef struct {
    uint32_t gps_ms_of_week;
    uint16_t gps_week;
} chronos_strt;

typedef struct {
    uint32_t gps_ms_of_week;
    uint8_t status;
} chronos_heab;

typedef struct {
    uint32_t gps_ms_of_week;
    double  x;
    double  y;
    double  z;
    double heading;
    double  lon_speed;
    double  lat_speed;
    double  lon_acc;
    double  lat_acc;
    uint8_t  direction; // [ 0 : Forward, 1 : Backward, 2 : Unavailable ]
    uint8_t  status;    // [ 0 : Off, 1 : Init, 2 : Armed, 3 : Disarmed, 4 : Running, 5 : Postrun, 6 : Remote controlled ]
    uint8_t  rdyToArm;  // [ 0 : Not ready, 1 : Ready, 2 : Unavailable ]
    uint8_t  error;     // Each bit represents an error status:
                        // [AbortReq, BrokeGeoFence, PoorPosAccuracy, EngineFault, BatFault, OtherObjError, Vendor, Vendor]
    uint8_t sender_id;
} chronos_monr;

typedef struct {
    int16_t speed;
    int16_t steering;
    ISO_UNIT_TYPE steeringUnit;
    ISO_UNIT_TYPE speedUnit;
    uint8_t command;             // Manoeuvre command (MANOEUVRE_NONE = 0, MANOEUVRE_BACK_TO_START = 3)
} chronos_rcmm;

typedef struct {
    uint16_t actionID;
    uint32_t executeTime;
} chronos_EXAC;

typedef struct {
    uint16_t actionID;
    uint16_t actionType;
    uint32_t actionTypeParam1;
    uint32_t actionTypeParam2;
    uint32_t actionTypeParam3;
} chronos_ACCM;

typedef struct {
    uint16_t triggerID;
    uint16_t triggerType;
    uint32_t triggerTypeParam1;
} chronos_TRCM;


typedef struct {
    uint16_t triggerID;
} chronos_CATA;

typedef struct {
    uint32_t sync_point;
    uint32_t stop_time;
} chronos_sypm;

typedef struct {
    uint64_t time_est;
} chronos_mtsp;

typedef struct {
    uint8_t status;
} chronos_init_sup;

#define PROTOCOL_VERSION 2

// Chronos messaging

// SYNC Word
#define ISO_SYNC_WORD                   0x7E7E
#define ISO_PART_SYNC_WORD              0x7E

// ISO Message Types
#define ISO_MSG_TRAJ                    0x0001
#define ISO_MSG_OSEM                    0x0002
#define ISO_MSG_OSTM                    0x0003
#define ISO_MSG_STRT                    0x0004
#define ISO_MSG_HEAB                    0x0005
#define ISO_MSG_MONR                    0x0006
#define ISO_MSG_RCMM                    0x000A

#define ISO_MSG_TRCM                    0x0011
#define ISO_MSG_ACCM                    0x0012
#define ISO_MSG_TREO                    0x0013
#define ISO_MSG_EXAC                    0x0014
#define ISO_MSG_CATA                    0x0015
#define ISO_MSG_OPRO                    0xA100

//#define ISO_MSG_OPRO                    0x000B
//#define ISO_MSG_OPRO_TO_OBJECT          0x0016

#define ISO_MSG_INIT_SUP                0xA102


// ISO Value Types
#define ISO_VALUE_ID_LAT                0x0020
#define ISO_VALUE_ID_LON                0x0021
#define ISO_VALUE_ID_ALT                0x0022
#define ISO_VALUE_ID_DateISO8601        0x0004
#define ISO_VALUE_ID_GPS_WEEK           0x0003
#define ISO_VALUE_ID_GPS_SEC_OF_WEEK    0x0002
#define ISO_VALUE_ID_MAX_WAY_DEV        0x0070
#define ISO_VALUE_ID_MAX_LATERAL_DEV    0x0072
#define ISO_VALUE_ID_MIN_POS_ACCURACY   0x0074
#define ISO_VALUE_ID_STATE_CHANGE_REQ   0x0064
#define ISO_VALUE_ID_DELAYED_START      0x0001
#define ISO_VALUE_ID_REL_TIME           0x0001
#define ISO_VALUE_ID_X_POS              0x0010
#define ISO_VALUE_ID_Y_POS              0x0011
#define ISO_VALUE_ID_Z_POS              0x0012
#define ISO_VALUE_ID_HEADING            0x0030
#define ISO_VALUE_ID_LONG_SPEED         0x0040
#define ISO_VALUE_ID_LAT_SPEED          0x0041
#define ISO_VALUE_ID_LONG_ACC           0x0050
#define ISO_VALUE_ID_LAT_ACC            0x0051
#define ISO_VALUE_ID_CURVATURE          0x0052
#define ISO_VALUE_ID_IP_ADDRESS         0x0053
#define ISO_VALUE_ID_OBJECT_TYPE        0x0054
#define ISO_VALUE_ID_OPERATION_MODE     0x0055
#define ISO_VALUE_ID_MASS               0x0056
#define ISO_VALUE_ID_ACTOR_TYPE         0x0057
#define ISO_VALUE_ID_TRANSMITTER_ID     0x0058
#define ISO_VALUE_ID_MONR_STRUCT        0x0080
#define ISO_VALUE_ID_HEAB_STRUCT        0x0090

//ACCM
#define ISO_VALUE_ID_ACTION_ID          0x0002
#define ISO_VALUE_ID_ACTION_TYPE        0x0003
#define ISO_VALUE_ID_ACTION_TYPE_PARAM1    0x00A1
#define ISO_VALUE_ID_ACTION_TYPE_PARAM2    0x00A2
#define ISO_VALUE_ID_ACTION_TYPE_PARAM3    0x00A3
#define ISO_ACTION_TYPE_MISC_DIGITAL_OUT 0x00E0
#define ISO_ACTION_TYPE_PARAM_SET_FALSE 0x0000
#define ISO_ACTION_TYPE_PARAM_SET_TRUE 0x0001

//TRCM
#define ISO_VALUE_ID_TRIGGER_ID          0x0001
#define ISO_VALUE_ID_TRIGGER_TYPE        0x0002
#define ISO_VALUE_ID_TRIGGER_TYPE_PARAM1    0x00A1
#define ISO_VALUE_ID_TRIGGER_TYPE_PARAM2    0x00A2
#define ISO_VALUE_ID_TRIGGER_TYPE_PARAM3    0x00A3

//EXAC
#define ISO_VALUE_ID_ACTION_ID          0x0002
#define ISO_VALUE_ID_EXECUTE_TIME       0x0003

//ACCM
#define ISO_VALUE_ID_ACTION_ID          0x0002
#define ISO_VALUE_ID_ACTION_TYPE        0x0003
#define ISO_VALUE_ID_ACTION_TYPE_PARAM1    0x00A1
#define ISO_VALUE_ID_ACTION_TYPE_PARAM2    0x00A2
#define ISO_VALUE_ID_ACTION_TYPE_PARAM3    0x00A3
#define ISO_ACTION_TYPE_MISC_DIGITAL_OUT 0x00E0
#define ISO_ACTION_TYPE_PARAM_SET_FALSE 0x0000
#define ISO_ACTION_TYPE_PARAM_SET_TRUE 0x0001

//TRCM
#define ISO_VALUE_ID_TRIGGER_ID          0x0001
#define ISO_VALUE_ID_TRIGGER_TYPE        0x0002
#define ISO_VALUE_ID_TRIGGER_TYPE_PARAM1    0x00A1
#define ISO_VALUE_ID_TRIGGER_TYPE_PARAM2    0x00A2
#define ISO_VALUE_ID_TRIGGER_TYPE_PARAM3    0x00A3

//EXAC
#define ISO_VALUE_ID_ACTION_ID          0x0002
#define ISO_VALUE_ID_EXECUTE_TIME       0x0003

#define ISO_VALUE_ID_TRAJECTORY_ID      0x0101
#define ISO_VALUE_ID_TRAJECTORY_NAME    0x0102
#define ISO_VALUE_ID_TRAJECTORY_VERSION 0x0103

#define ISO_VALUE_ID_INIT_SUP_STATUS    0x0200
#define AUX_VALUE_ID_OBJECT_ID          0xA000

//OSEM
#define ISO_VALUE_ID_OSEM_TRANSMITTER_ID 0x0010

//RCMM
#define ISO_VALUE_ID_RCMM_CONTROL_STATUS         0x0001
#define ISO_VALUE_ID_RCMM_SPEED_METER_PER_SECOND 0x0011
#define ISO_VALUE_ID_RCMM_STEERING_ANGLE         0x0012
#define ISO_VALUE_ID_RCMM_STEERING_PERCENTAGE    0x0031
#define ISO_VALUE_ID_RCMM_SPEED_PERCENTAGE       0x0032
#define ISO_VALUE_ID_RCMM_CONTROL                0xA201

// OPRO
#define ISO_VALUE_ID_OPRO_OBJECT_TYPE 0x0100
#define ISO_VALUE_ID_OPRO_ACTOR_TYPE 0x0101
#define ISO_VALUE_ID_OPRO_OPERATION_MODE 0x0102
#define ISO_VALUE_ID_OPRO_MASS 0x0103
#define ISO_VALUE_ID_OPRO_OBJECT_LENGTH_X 0x0104
#define ISO_VALUE_ID_OPRO_OBJECT_LENGTH_Y 0x0105
#define ISO_VALUE_ID_OPRO_OBJECT_LENGTH_Z 0x0106
#define ISO_VALUE_ID_OPRO_POSITION_DISPLACEMENT_X 0x0107
#define ISO_VALUE_ID_OPRO_POSITION_DISPLACEMENT_Y 0x0108
#define ISO_VALUE_ID_OPRO_POSITION_DISPLACEMENT_Z 0x0109

// ************************* Type definitions according ISO protocol specification *******************************
// Predefined integer values with special meaning
#define ISO_SPEED_UNAVAILABLE_VALUE (-32768)
#define ISO_SPEED_ONE_METER_PER_SECOND_VALUE 100.0
#define ISO_STEERING_ANGLE_ONE_DEGREE_VALUE 100.0
#define ISO_STEERING_ANGLE_MAX_VALUE_DEG 18000
#define ISO_STEERING_ANGLE_MIN_VALUE_DEG (-18000)
#define ISO_STEERING_ANGLE_MAX_VALUE_RAD M_PI
#define ISO_STEERING_ANGLE_UNAVAILABLE_VALUE 18001
#define ISO_MAX_VALUE_PERCENTAGE 100
#define ISO_MIN_VALUE_PERCENTAGE (-100)

// Object states
#define ISO_OBJECT_STATE_OFF 0x00
#define ISO_OBJECT_STATE_INIT 0x01
#define ISO_OBJECT_STATE_ARMED 0x02
#define ISO_OBJECT_STATE_DISARMED 0x03
#define ISO_OBJECT_STATE_RUNNING 0x04
#define ISO_OBJECT_STATE_POSTRUN 0x05
#define ISO_OBJECT_STATE_REMOTECONTROL 0x06
#define ISO_OBJECT_STATE_ABORT 0x07

class ChronosComm : public QObject
{
    Q_OBJECT
public:
    explicit ChronosComm(QObject *parent = nullptr);
    bool startObject(QHostAddress addr = QHostAddress::Any);
    bool startSupervisor(QHostAddress addr = QHostAddress::Any);
    bool connectAsServer(QString address);
    void closeConnection();
    COMM_MODE getCommMode();
    void sendTraj(chronos_traj traj);
    void sendHeab(chronos_heab heab);
    void sendOsem(chronos_osem osem);
    void sendOstm(chronos_ostm ostm);
    void sendOpro(chronos_opro opro);
    void sendStrt(chronos_strt strt);
    void sendMonr(chronos_monr monr);
    void sendInitSup(chronos_init_sup init_sup);

    quint8 transmitterId() const;
    void setTransmitterId(const quint8 &transmitterId);

    static quint32 gpsMsOfWeek();
    static quint32 gpsWeek();
    static quint32 gpsMsOfWeekToUtcToday(quint64 time);

signals:
    void connectionChanged(bool connected, QString address);
    void trajRx(chronos_traj traj);
    void heabRx(chronos_heab heab);
    void osemRx(chronos_osem osem);
    void oproRx(chronos_opro opro);
    void ostmRx(chronos_ostm ostm);
    void strtRx(chronos_strt strt);
    void monrRx(chronos_monr monr);
    void insupRx(chronos_init_sup init_sup);
    void rcmmRx(chronos_rcmm rcmm);

	void heabTimeOut();
    void rcmmTimeOut();

public slots:
	void startHeabLastHeabReceivedTimer();
	void checkLastHeabRestart();
    void checkRcmmLastRecievedTimer();

private slots:
    void tcpRx(QByteArray data);
    void tcpConnectionChanged(bool connected, QString address);
    void readPendingDatagrams();

    void tcpInputConnected();
    void tcpInputDisconnected();
    void tcpInputDataAvailable();
    void tcpInputError(QAbstractSocket::SocketError socketError);

private:
    TcpServerSimple *mTcpServer;
    QTcpSocket *mTcpSocket;
    QUdpSocket *mUdpSocket;
    GPIO* mGpioControl;
    QHostAddress mUdpHostAddress;
    quint16 mUdpPort;
    quint8 mTransmitterId;
    quint8 mChronosSeqNum;
    COMM_MODE mCommMode;

    int mTcpState;
    quint16 mTcpType;
    quint32 mTcpLen;
    quint16 mTcpChecksum;
    VByteArrayLe mTcpData;

	QElapsedTimer mLastHeabReceivedTimer;
	QTimer *mLastHeabTimer;

    QElapsedTimer mLastRcmmReceivedElapsedTimer;
    QTimer *mRemoteControlStateTimer;

    void mkChronosHeader(VByteArrayLe &vb,
                         quint8 transmitter_id,
                         quint8 sequence_num, // per object sequence num
                         bool ack_req,
                         quint8 protocol_ver, // 7 bits
                         quint16 message_id);
    void appendChronosChecksum(VByteArrayLe &vb);
    bool decodeMsg(quint16 type, quint32 len, QByteArray payload, uint8_t sender_id);
    void sendData(QByteArray data, bool isUdp);
    void configureTrigger(chronos_TRCM trcm);
    void configureAction(chronos_ACCM accm);
    void executeAction(chronos_EXAC exac);
    QList<std::pair<quint16,GPIO::PinOperation_t>> actionPinConnections;
};

#endif // CHRONOSCOMM_H
