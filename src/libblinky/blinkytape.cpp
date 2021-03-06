#include "blinkytape.h"

#include "blinkycontrollerinfo.h"
#include "blinkytapeuploader.h"
#include "eightbyeightuploader.h"
#include "blinkypendantuploader.h"
#include "lightbuddyuploader.h"

#include "avr109firmwareloader.h"
#include "esp8266firmwareloader.h"
#include "usbdeviceidentifier.h"

#include <QDebug>
#include <QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>

/// Interval between scans to see if the device is still connected
#define CONNECTION_SCANNER_INTERVAL 100

// TODO: Put this somewhere better?
#define PENDANT_MAX_PIXELS 10

#define RESET_TIMER_TIMEOUT 500

#define RESET_MAX_TRIES 3

#define MIN_UPDATE_INTERVAL 3

#define WRITE_BUSY_DELAY 2
#define WRITE_CHUNK_DELAY 4
#define CHUNK_SIZE 300


BlinkyTape::BlinkyTape(QSerialPortInfo info, QObject *parent) :
    BlinkyController(parent),
    serialInfo(info)
{
    serial = new QSerialPort(this);
    serial->setSettingsRestoredOnClose(false);

    connect(serial, &QSerialPort::errorOccurred,
            this, &BlinkyTape::handleError);

    connect(serial, &QSerialPort::readyRead,
            this, &BlinkyTape::handleReadData);

    connect(serial, &QSerialPort::baudRateChanged,
            this, &BlinkyTape::handleBaudRateChanged);

    resetTimer.setSingleShot(true);
    connect(&resetTimer, &QTimer::timeout,
            this, &BlinkyTape::resetTimer_timeout);

    // TODO: Consider spawning new timers for this instead of keeping a global one
    serialWriteTimer.setSingleShot(true);
    connect(&serialWriteTimer, &QTimer::timeout,
            this, &BlinkyTape::sendChunk);

    // On Windows, we don't get notified if the tape was disconnected, so we have to check peroidically
#if defined(CONNECTION_SCANNER_TIMER)
    connectionScannerTimer.setInterval(CONNECTION_SCANNER_INTERVAL);
    connect(&connectionScannerTimer, QTimer::timeout,
            this, &BlinkyTape::connectionScannerTimer_timeout);
#endif
}

QString BlinkyTape::getName() const
{
    return QString("BlinkyTape");
}

void BlinkyTape::handleError(QSerialPort::SerialPortError error)
{
    // The serial library appears to emit an extraneous SerialPortError
    // when open() is called. Just ignore it.
    if (error == QSerialPort::NoError)
        return;

    QString errorString = serial->errorString();

    if (error == QSerialPort::ResourceError)
        qCritical() << "Serial resource error, device unplugged?" << errorString;
    else
        qCritical() << "Unrecognized serial error:" << errorString;

    close();
}

void BlinkyTape::resetTimer_timeout()
{
    if (!isConnected())
        return;

    qDebug() << "Hit reset timer";

    if (resetTriesRemaining < 1) {
        qCritical() << "Reset timer maximum tries reached, failed to reset tape";
        return;
    }

    serial->setBaudRate(QSerialPort::Baud1200);

    // setBaudRate() doesn't seem to be reliable if called too quickly after the port
    // is opened. In this case,
    resetTimer.start(RESET_TIMER_TIMEOUT);

    resetTriesRemaining--;
}

void BlinkyTape::sendChunk()
{
    // If the last bit didn't get written yet, try again shortly.
    if (serial->bytesToWrite() > 0) {
        qDebug() << "Busy, trying again shortly";
        serialWriteTimer.start(WRITE_BUSY_DELAY);
        return;
    }

    while (!chunks.isEmpty()) {
        QByteArray chunk = chunks.takeFirst();

        int writeLength = serial->write(chunk);
        if (writeLength != chunk.length()) {
            qCritical() << "Error writing all the data out, expected:" << chunk.length()
                        << ", wrote:" << writeLength;
        }
        serial->flush();

        if (!chunks.isEmpty()) {
            serialWriteTimer.start(WRITE_CHUNK_DELAY);
            return;
        }
    }
}

#if defined(CONNECTION_SCANNER_TIMER)
void BlinkyTape::connectionScannerTimer_timeout()
{
    // If we are already disconnected, disregard.
    if (!isConnected()) {
        connectionScannerTimer.stop();
        return;
    }

    // Check if our serial port is still available.
    for (BlinkyControllerInfo &info : BlinkyControllerInfo::availableControllers()) {
        // If we get a match, reset the timer and return.
        // We consider it a match if the port is the same
        if (info.resourceName() == serial->portName())
            return;
    }

    // We seem to have lost our port, bail
    close();
}
#endif

bool BlinkyTape::open()
{
    if (isConnected()) {
        qCritical() << "Already connected to a device";
        return false;
    }

    qDebug() << "Connecting to device on " << serialInfo.portName();

    serial->setPortName(serialInfo.portName());
    serial->setBaudRate(QSerialPort::Baud115200);

    if (!serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Could not connect to device. Error: " << serial->error()
                 << serial->errorString();
        return false;
    }

    resetTriesRemaining = 0;

    emit(connectionStatusChanged(true));

#if defined(CONNECTION_SCANNER_TIMER)
    // Schedule the connection scanner
    connectionScannerTimer.start();
#endif
    return true;
}

void BlinkyTape::close()
{
    if (isConnected())
        serial->close();

    resetTriesRemaining = 0;

    emit(connectionStatusChanged(isConnected()));
}

void BlinkyTape::handleReadData()
{
    // Discard any data we get back from the BlinkyTape
    serial->readAll();
}

void BlinkyTape::handleBaudRateChanged(qint32 baudRate, QSerialPort::Directions)
{
    if (baudRate == QSerialPort::Baud115200) {
        qDebug() << "Baud rate updated to 115200!";
    } else if (baudRate == QSerialPort::Baud1200 && resetTriesRemaining > 0) {
        qDebug() << "Baud rate updated to 1200bps, closing!";

        resetTimer.stop();
        close();
    } else if (baudRate == QSerialPort::Baud1200) {
        qDebug() << "Baud rate updated to 1200bps spuriously";
    }
}

bool BlinkyTape::isConnected()
{
    return serial->isOpen();
}

void BlinkyTape::sendUpdate(QByteArray ledData)
{
    if (!isConnected()) {
        qCritical() << "Strip not connected, not sending update!";
        return;
    }

    // If there is data pending to send, skip this update to prevent overflowing
    // the buffer.
    if (!chunks.isEmpty()) {
        qDebug() << "Output data still in buffer, dropping this update frame";
        return;
    }

    // Trim anything that's 0xff
    for (int i = 0; i < ledData.length(); i++) {
        if (ledData[i] == (char)255)
            ledData[i] = (char)254;
    }

    // Append an 0xFF to signal the flip command
    ledData.append((char)0xFF);

    for (int chunk = 0; chunk *CHUNK_SIZE < ledData.length(); chunk++) {
        int chunkLength = CHUNK_SIZE;
        if ((chunk+1)*CHUNK_SIZE > ledData.length())
            chunkLength = ledData.length() - chunk*CHUNK_SIZE;

        QByteArray chunkData = ledData.mid(chunk*CHUNK_SIZE, chunkLength);
        chunks.append(chunkData);

// qDebug() << "size=" << ledData.length() << " chunk=" << chunk << " position=" << chunk*CHUNK_SIZE << " length=" << chunkLength;
// qDebug() << chunkData.toHex();
    }

// qDebug() << "Total chunks:" << chunks.size();
    sendChunk();
}

bool BlinkyTape::getPortInfo(QSerialPortInfo &info)
{
    if (!isConnected())
        return false;

    info = serialInfo;
    return true;
}

void BlinkyTape::reset()
{
    if (!isConnected() || serial->error() != QSerialPort::NoError)
        return;
    qDebug() << serial->error();

    qDebug() << "Attempting to reset device";

    resetTriesRemaining = RESET_MAX_TRIES;
    resetTimer_timeout();
}

bool BlinkyTape::getPatternUploader(QPointer<BlinkyUploader> &uploader)
{
    if (!isConnected())
        return false;

    for(UsbDeviceIdentifier identifier : patternUploaders) {
        if(identifier.matches(serialInfo)) {
            if(identifier.name == "BlinkyTapeUploader") {
                uploader = new BlinkyTapeUploader(parent());
                return true;
            }
            else if(identifier.name == "BlinkyPendantUploader") {
                uploader = new BlinkyPendantUploader(parent());
                return true;
            }
            else if(identifier.name == "LightBuddyUploader") {
                uploader = new LightBuddyUploader(parent());
                return true;
            }
            else if(identifier.name == "EightByEightUploader") {
                uploader = new EightByEightUploader(parent());
                return true;
            }
            else {
                return false;
            }
        }
    }

    return false;
}

bool BlinkyTape::getFirmwareLoader(QPointer<FirmwareLoader> &loader)
{
    if (!isConnected())
        return false;

    for(UsbDeviceIdentifier identifier : firmwareLoaders) {
        if(identifier.matches(serialInfo)) {
            if(identifier.name == "Avr109FirmwareLoader") {
                loader = new Avr109FirmwareLoader(parent());
                return true;
            }
            else if(identifier.name == "Esp8266FirmwareLoader") {
                loader = new Esp8266FirmwareLoader(parent());
                return true;
            }
            else {
                return false;
            }
        }
    }

    return false;
}
