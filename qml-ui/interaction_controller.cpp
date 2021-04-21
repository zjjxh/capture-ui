//==============================================================================
// Includes
//==============================================================================
#include "interaction_controller.h"
#include "video-capture.h"

#include <QRunnable>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QApplication>
#include <QScreen>
#include <QWidget>
#include <QDesktopWidget>

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
	m_videorgb = parent()->findChild<QQuickItem *>("videoItem");
	Q_ASSERT(NULL != m_videorgb);
	//m_Detailbtn = parent()->findChild<QQuickItem *>("detail-btn");
	//Q_ASSERT(NULL != m_Detailbtn);
	QObject::connect(m_Capturebtn, SIGNAL(capture_image(int, QString, int, bool, QString)), this,
			 SLOT(capture_image(int, QString, int, bool, QString)));
	QObject::connect(m_Freshbtn, SIGNAL(get_fresh(int)), this,
			 SLOT(get_fresh(int)));
	QObject::connect(m_videorgb, SIGNAL(get_videorgb()), this,
			 SLOT(get_videorgb()));
	QObject::connect(m_videorgb, SIGNAL(video_press(int, int)), this,
			 SLOT(video_press(int, int)));
	QObject::connect(m_videorgb, SIGNAL(video_release(int, int, int, int, int, int)), this,
			 SLOT(video_release(int, int, int, int, int, int)));

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

//ugly...but quick to impl
static int x1 = 0;
static int y1 = 0;
static int cx1 = 0;
static int cy1 = 0;
void InteractionController::get_fresh(int card){
	fresh_capture(card, nullptr, 0, 0, 0, 0, 0, 0);
	QMetaObject::invokeMethod(m_Freshbtn, "fresh_meta", Q_ARG(QVariant, get_capture_fourcc()),
				  Q_ARG(QVariant, get_capture_width()), Q_ARG(QVariant, get_capture_height()),
				  Q_ARG(QVariant, CS_NAME[get_capture_cs_id()]));
	x1 = y1 = cx1 = cy1 = 0;
	QString text = "RGB: unknown";
	QMetaObject::invokeMethod(m_videorgb, "videorgb_meta", Q_ARG(QVariant, text));
	text = "region: default";
	QMetaObject::invokeMethod(m_videorgb, "picked_region_meta", Q_ARG(QVariant, text));
}

void InteractionController::video_press(int x, int y)
{
	qDebug() << "enter into video_press" << x << y;
}

void InteractionController::video_release(int w0, int h0, int x, int y, int w1, int h1)
{
	QString text = "region: default";
	double r = 0.0;
	int w = get_capture_width();
	int h = get_capture_height();
	//int w = 4000;
	//int h = 3000;
	qDebug() << "enter into video_release" << w << h << w0 << h0 << x << y << w1 << h1;
	if (w <= 0 || h <= 0 || w1 <= 0 || h1 <= 0 || x <= 0 || y <= 0)
		goto reset;
	if (h * w0 < h0 * w) {
		r = w / w0;
		x1 = x;
		y1 = y + (h * w0 / (double)w - h0) / 2;

	} else {
		r = h / h0;
		x1 = x + (w * h0 / (double)h - w0) / 2;
		y1 = y;
	}
	x1 = x1 * r;
	y1 = y1 * r;
	cx1 = w1 * r;
	cy1 = h1 * r;

	if (x1 < 0) {
		cx1 += x1;
		x1 = 0;
	}
	if (y1 < 0) {
		cy1 += y1;
		y1 = 0;
	}
	if (x1 >= w || y1 >= h)
		goto reset;

	if (x1 + cx1 > w)
		cx1 = w - x1;
	if (y1 + cy1 > h)
		cy1 = h - y1;
	if (cx1 <= 0 || cy1 <= 0)
		goto reset;

final:
	if (cx1 != 0)
		text = QString("region: %1, %2\nsize: %3, %4").arg(x1).arg(y1).arg(cx1).arg(cy1);
	qDebug() << x1 << y1 << cx1 << cy1;
	qDebug() << text;
	QMetaObject::invokeMethod(m_videorgb, "picked_region_meta", Q_ARG(QVariant, text));
	return;
reset:
	qDebug() << "reset region";
	x1 = y1 = cx1 = cy1 = 0;
	goto final;
}

void InteractionController::get_videorgb() {
	int x = QCursor::pos().x();
	int y = QCursor::pos().y();
	int mousedPressed_R = 0;
	int mousedPressed_G = 0;
	int mousedPressed_B = 0;
	QString text = "RGB: failed";
	QScreen *screen = QGuiApplication::primaryScreen();
	QPixmap pixmap = screen->grabWindow((QApplication::desktop())->winId(), x, y, 1, 1);
	if (!pixmap.isNull()) {
		QImage image = pixmap.toImage();
		if (!image.isNull()) {
			if (image.valid(0, 0)) {
				QColor color = image.pixel(0, 0);
				mousedPressed_R = color.red();
				mousedPressed_G = color.green();
				mousedPressed_B = color.blue();
				text = QString("RGB: %1, %2, %3").arg(mousedPressed_R).arg(mousedPressed_G).arg(mousedPressed_B);
				qDebug() << text;
			}
		}
	}
	QMetaObject::invokeMethod(m_videorgb, "videorgb_meta", Q_ARG(QVariant, text));
}

/*void InteractionController::get_detail(){
    QMetaObject::invokeMethod(m_Detailbtn, "detail_meta", Q_ARG(QVariant, get_capture_inputinfo()));
}*/

void InteractionController::capture_image(int card, QString base, int cnt, bool need_bmp, QString cslabel) {
	(void)cslabel;
	qDebug() << "capture_image" << base;
	if (!QDir().mkpath(base))
		return;
	qDebug() << "capture_image mkdir " << base << "pass";

	fresh_capture(card, base.toLatin1().data(), cnt, need_bmp, x1, y1, cx1, cy1);
#if 0
	QString raw_file = base + "0." + get_capture_fourcc();
	qDebug() << raw_file;
	if (QFile(raw_file).exists()) {
		QFile file(base+"meta.txt");
		file.open(QIODevice::ReadWrite|QIODevice::Text);
		QTextStream in(&file);
		in<<("custom-colorspace:"+cslabel)<<"\n";
		in<<("detect-colorspace:"+QString(CS_NAME[get_capture_cs_id()]))<<"\n";
		in<<("width:"+QString::number(get_capture_width()))<<"\n";
		in<<("height:"+QString::number(get_capture_height()))<<"\n";
		in<<("pixel-format:"+QString(get_capture_fourcc()))<<"\n";
		file.close();
	}
#endif
	x1 = y1 = cx1 = cy1 = 0;
	QString text = "RGB: unknown";
	QMetaObject::invokeMethod(m_videorgb, "videorgb_meta", Q_ARG(QVariant, text));
	text = "region: default";
	QMetaObject::invokeMethod(m_videorgb, "picked_region_meta", Q_ARG(QVariant, text));
	QMetaObject::invokeMethod(m_Capturebtn, "image_meta", Q_ARG(QVariant, get_capture_fourcc()),
				  Q_ARG(QVariant, get_capture_width()), Q_ARG(QVariant, get_capture_height()),
				  Q_ARG(QVariant, CS_NAME[get_capture_cs_id()]));
}
