//==============================================================================
// Project: QtQuickPlayer
// Author: Ivan Vasilev <ivan.v.vasilev at Gmail>
//
// A simple application demostrating QtQuick(QML) integration with gstreamer
//
// interaction_controller.cpp - a class to handle QML<-->C++ interaction -
//  mainly responding to user commands for now
//
//==============================================================================

//==============================================================================
// Includes
//==============================================================================
#include "interaction_controller.h"

#include <QRunnable>

class SetGstState : public QRunnable {
public:
	SetGstState(GstElement *, GstState);
	~SetGstState();

	void run();

private:
	GstElement *pipeline_;
	GstState state_;
};

SetGstState::SetGstState(GstElement * pipeline, GstState state) {
	this->pipeline_ = pipeline ? static_cast<GstElement *> (gst_object_ref (pipeline)) : NULL;
	this->state_ = state;
}

SetGstState::~SetGstState() {
	if (this->pipeline_)
		gst_object_unref(this->pipeline_);
}

void
SetGstState::run () {
	if (this->pipeline_)
		gst_element_set_state(this->pipeline_, this->state_);
}

//==============================================================================
// InteractionController Exported methods
//==============================================================================
InteractionController::InteractionController(QQuickWindow *window, GstElement *gstPlayer, GstElement *snapShot, bool autoplay) :
QObject(window),
m_GstPlayer(gstPlayer),
m_snapshot(snapShot),
m_RootObject(window) {
	Q_ASSERT(NULL != m_GstPlayer);
	gst_object_ref_sink(m_GstPlayer);
#if 0
	// set-up Signal->Slot connections
	m_PlayPauseControl = parent()->findChild<QQuickItem *>("playPauseControl");
	Q_ASSERT(NULL != m_PlayPauseControl);
	QObject::connect(m_PlayPauseControl, SIGNAL(playStateChanged()), this, SLOT(PlayPauseSlot()));

	QQuickItem * fullscreenControl = parent()->findChild<QQuickItem *>("fullscreenControl");
	Q_ASSERT(NULL != fullscreenControl);
	QObject::connect(fullscreenControl, SIGNAL(fullScreenClicked()), this, SLOT(FullScreenSlot()));

	if (autoplay)
		QMetaObject::invokeMethod(m_PlayPauseControl, "play");
#endif
    m_RootObject->scheduleRenderJob (new SetGstState(m_GstPlayer, GST_STATE_PLAYING),
                     QQuickWindow::BeforeSynchronizingStage);
}

InteractionController::~InteractionController() {
	// clean up when the window(our parent) is closed
	gst_element_set_state(m_GstPlayer, GST_STATE_NULL);
	gst_object_unref(m_GstPlayer);
}

void InteractionController::PlayPauseSlot() {
#if 0
	if (true == m_PlayPauseControl->property("playing")) {
		qDebug("playing...");
		m_RootObject->scheduleRenderJob (new SetGstState(m_GstPlayer, GST_STATE_PLAYING),
						 QQuickWindow::BeforeSynchronizingStage);
	} else {
		qDebug("pausing...");
		m_RootObject->scheduleRenderJob (new SetGstState(m_GstPlayer, GST_STATE_PAUSED),
						 QQuickWindow::BeforeSynchronizingStage);
	}
#endif
}

void InteractionController::FullScreenSlot() {
#if 0
	qDebug("entering fullscreen...");
	if (QQuickWindow::FullScreen != ((QQuickWindow *)parent())->visibility()) {
		((QQuickWindow *)parent())->showFullScreen();
	} else {
		((QQuickWindow *)parent())->showNormal();
	}
#endif
	qDebug("snapshot event...");
}
