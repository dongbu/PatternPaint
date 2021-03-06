#ifndef BLINKYUPLOADER_H
#define BLINKYUPLOADER_H

#include "libblinkyglobal.h"

#include "patternwriter.h"

#include <QObject>
#include <QList>

class BlinkyController;

/// This is an re-entreant version of an pattern uploader.
/// Each task in the upload process is broken into a single state, and the state
/// machine is driven forward by receiving events from the connected serial port,
/// or in the case of a timeout, from the timeout timer.
///
/// While the upload process is underway, it will send periodic progress updates
/// via the progressUpdate() signal.
class LIBBLINKY_EXPORT BlinkyUploader : public QObject
{
    Q_OBJECT

public:
    BlinkyUploader(QObject *parent = 0) :
        QObject(parent)
    {
    }

    /// Start an upload, using the passed blinkytape as a launching point
    /// Note that the blinkytape will be disconnected during the upload process,
    /// and will need to be reconnected manually afterwards.
    /// @param tape BlinkyTape to upload to (must already be connected)
    /// @param patternWriters Patterns to upload to the BlinkyTape
    virtual bool storePatterns(BlinkyController &controller, QList<PatternWriter> &patternWriters) = 0;

    /// Get a string describing the last error, if any.
    virtual QString getErrorString() const = 0;

    virtual QList<PatternWriter::Encoding> getSupportedEncodings() const = 0;

signals:
    /// Sends an update about the upload progress, from 0 to 100
    void progressChanged(int progress);

    /// Send an update for the text
    void setText(QString textLabel);

    /// Sends a signal at end of upload to report the result.
    void finished(bool result);

public slots:
    /// Request that the upload process be stopped
    virtual void cancel() = 0;
};

#endif // BLINKYUPLOADER_H
