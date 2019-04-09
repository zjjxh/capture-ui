#ifndef GSTBUSPOLLER_H_
#define GSTBUSPOLLER_H_

//==============================================================================
// Includes
//==============================================================================

// QT Includes
#include <QObject>
#include <QRunnable>
#include <QBasicTimer>
#include <QTimerEvent>

// gstreamer includes
#include "gst/gst.h"

class GstBusPoller : public QObject
{
public:
	GstBusPoller(GstBus *bus, QString name);
	~GstBusPoller();

private:
	QString m_Name;
	GstBus *m_Bus;
	QBasicTimer m_Timer;

	virtual void timerEvent(QTimerEvent *event);
};

#endif // GSTBUSPOLLER_H
