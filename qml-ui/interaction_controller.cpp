//==============================================================================
// Includes
//==============================================================================
#include "interaction_controller.h"
#include "video-capture.h"

#include <QRunnable>
#include <QFile>
#include <QTextStream>

class SetGstState : public QRunnable {
public:
    SetGstState(GstElement *, GstElement *, InteractionController *, QString);
	~SetGstState();

	void run();
private:
    GstElement *m_pipeline;
    GstElement *m_gstSrc;
    QString m_src;
    InteractionController *m_controller;
};

SetGstState::SetGstState(GstElement *pipeline, GstElement *gstSrc, InteractionController *controller, QString src) {
    Q_ASSERT(pipeline);
    m_pipeline = static_cast<GstElement *> (gst_object_ref(pipeline));
    m_gstSrc = gstSrc;
    m_controller = controller;
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
    m_controller->get_fresh(m_controller->m_SelectSrc->property("currentIndex").toInt());
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
    m_Freshbtn = parent()->findChild<QQuickItem *>("fresh-btn");
    Q_ASSERT(NULL != m_Freshbtn);
    //m_Detailbtn = parent()->findChild<QQuickItem *>("detail-btn");
    //Q_ASSERT(NULL != m_Detailbtn);
    QObject::connect(m_Capturebtn, SIGNAL(capture_image(int, QString, int, bool, QString)), this,
                     SLOT(capture_image(int, QString, int, bool, QString)));
    QObject::connect(m_Freshbtn, SIGNAL(get_fresh(int)), this,
                     SLOT(get_fresh(int)));
    //QObject::connect(m_Detailbtn, SIGNAL(get_detail()), this,
    //                 SLOT(get_detail()));
}

InteractionController::~InteractionController() {
	// clean up when the window(our parent) is closed
	gst_element_set_state(m_GstPlayer, GST_STATE_NULL);
	gst_object_unref(m_GstPlayer);
}

void InteractionController::select_src(QString src) {
    m_RootObject->scheduleRenderJob(new SetGstState(m_GstPlayer, m_GstSrc, this, src),
            QQuickWindow::BeforeSynchronizingStage);
}

void InteractionController::get_fresh(int card){
    fresh_capture(card, nullptr, 0, 0);
    QMetaObject::invokeMethod(m_Freshbtn, "fresh_meta", Q_ARG(QVariant, get_capture_fourcc()),
                              Q_ARG(QVariant, get_capture_width()), Q_ARG(QVariant, get_capture_height()),
                              Q_ARG(QVariant, CS_NAME[get_capture_cs_id()]));
}

/*void InteractionController::get_detail(){
    QMetaObject::invokeMethod(m_Detailbtn, "detail_meta", Q_ARG(QVariant, get_capture_inputinfo()));
}*/

void InteractionController::capture_image(int card, QString base, int cnt, bool need_bmp, QString cslabel) {
    fresh_capture(card, base.toLatin1().data(), cnt, need_bmp);
    QString raw_file = base + ".0." + get_capture_fourcc();
    qDebug() << raw_file;
    if (QFile(raw_file).exists()) {
        QFile file(base+".meta");
        file.open(QIODevice::ReadWrite|QIODevice::Text);
        QTextStream in(&file);
        in<<("custom-colorspace:"+cslabel)<<"\n";
        in<<("detect-colorspace:"+QString(CS_NAME[get_capture_cs_id()]))<<"\n";
        in<<("width:"+QString::number(get_capture_width()))<<"\n";
        in<<("height:"+QString::number(get_capture_height()))<<"\n";
        in<<("pixel-format:"+QString(get_capture_fourcc()))<<"\n";
        file.close();
    }
    QMetaObject::invokeMethod(m_Capturebtn, "image_meta", Q_ARG(QVariant, get_capture_fourcc()),
                              Q_ARG(QVariant, get_capture_width()), Q_ARG(QVariant, get_capture_height()),
                              Q_ARG(QVariant, CS_NAME[get_capture_cs_id()]));
}

