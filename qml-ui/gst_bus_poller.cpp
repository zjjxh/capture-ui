//==============================================================================
// Includes
//==============================================================================
#include "gst_bus_poller.h"

//==============================================================================
// GstBusPoller Exported methods
//==============================================================================
GstBusPoller::GstBusPoller(GstBus *bus, QString name) : QObject(), m_Name(name), m_Bus(bus) {
	g_object_ref(m_Bus);
	m_Timer.start(500, this);
}

GstBusPoller::~GstBusPoller() {
	m_Timer.stop();
	g_object_unref(m_Bus);
}

//==============================================================================
// GstBusPoller private methods
//==============================================================================
void GstBusPoller::timerEvent(QTimerEvent *event) {
	if (event->timerId() != m_Timer.timerId())
		return QObject::timerEvent(event);

	GstMessage *message;
	while (nullptr != (message = gst_bus_pop(m_Bus))) {
		switch (GST_MESSAGE_TYPE(message)) {
		case GST_MESSAGE_ERROR: {
			GError *err = nullptr;
			gchar *dbg_info = nullptr;

			gst_message_parse_error(message, &err, &dbg_info);
			g_printerr("%s:ERROR from element %s: %s\r\n", m_Name.toLatin1().data(), GST_OBJECT_NAME(message->src), err->message);
			g_printerr("%s:Debugging info: %s\r\n", m_Name.toLatin1().data(), (dbg_info) ? dbg_info : "none");
			g_error_free(err);
			g_free(dbg_info);
			break;
		}
		default:
			g_printerr("%s: Got Message with type: %s\r\n", m_Name.toLatin1().data(), gst_message_type_get_name(GST_MESSAGE_TYPE(message)));
			break;
		}
		gst_message_unref(message);
	}
}
