//==============================================================================
// Includes
//==============================================================================
#include "interaction_controller.h"
#include "video-capture.h"

#include <QRunnable>

class SetGstState : public QRunnable {
public:
    SetGstState(GstElement *, GstElement *, QString);
	~SetGstState();

	void run();
private:
    GstElement *m_pipeline;
    GstElement *m_gstSrc;
    QString m_src;
};

SetGstState::SetGstState(GstElement *pipeline, GstElement *gstSrc, QString src) {
    Q_ASSERT(pipeline);
    m_pipeline = static_cast<GstElement *> (gst_object_ref(pipeline));
    m_gstSrc = gstSrc;
    m_src = src;
}

SetGstState::~SetGstState() {
    gst_object_unref(m_pipeline);
}

void
SetGstState::run() {
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    qDebug() << "select input src" << m_src;
    g_object_set(m_gstSrc, "device", m_src.toLatin1().data(), nullptr);
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
}

//==============================================================================
// InteractionController Exported methods
//==============================================================================
InteractionController::InteractionController(QQuickWindow *window, GstElement *gstPlayer, GstElement *gstSrc) :
QObject(window),
m_GstPlayer(gstPlayer),
m_GstSrc(gstSrc),
m_RootObject(window) {
	Q_ASSERT(NULL != m_GstPlayer);
	gst_object_ref_sink(m_GstPlayer);
    // setup Signal->Slot connections.change-input-src
    m_SelectSrc = parent()->findChild<QQuickItem *>("video-input-src");
    Q_ASSERT(NULL != m_SelectSrc);
    QObject::connect(m_SelectSrc, SIGNAL(inputSrc(QString)), this, SLOT(select_src(QString)));
    m_SelectSrc->setProperty("currentIndex", 0);
    QMetaObject::invokeMethod(m_SelectSrc, "trigger");

    //setup catpure btn
    m_Capturebtn = parent()->findChild<QQuickItem *>("capture-btn");
    Q_ASSERT(NULL != m_Capturebtn);
    QObject::connect(m_Capturebtn, SIGNAL(capture_image(int, QString, int, bool)), this,
                     SLOT(capture_image(int, QString, int, bool)));
}

InteractionController::~InteractionController() {
	// clean up when the window(our parent) is closed
	gst_element_set_state(m_GstPlayer, GST_STATE_NULL);
	gst_object_unref(m_GstPlayer);
}

void InteractionController::select_src(QString src) {
    m_RootObject->scheduleRenderJob(new SetGstState(m_GstPlayer, m_GstSrc, src),
            QQuickWindow::BeforeSynchronizingStage);
}
void InteractionController::capture_image(int card, QString base, int cnt, bool need_bmp) {
    fresh_capture(card, base.toLatin1().data(), cnt, need_bmp);
    QMetaObject::invokeMethod(m_Capturebtn, "image_meta", Q_ARG(QVariant, get_capture_fourcc()),
                              Q_ARG(QVariant, get_capture_width()), Q_ARG(QVariant, get_capture_height()),
                              Q_ARG(QVariant, get_capture_cs_id()));
}

