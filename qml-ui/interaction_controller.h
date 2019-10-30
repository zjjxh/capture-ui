#ifndef INTERACTIONCONTROLLER_H_
#define INTERACTIONCONTROLLER_H_

//==============================================================================
// Includes
//==============================================================================
#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>

#include "gst/gst.h"

class SetGstState;

class InteractionController : public QObject
{
	Q_OBJECT
public:
    explicit InteractionController(QQuickWindow *window, GstElement *gstPlayer, GstElement *gstSrc);
	~InteractionController();
signals:
	// none at that point
public slots:
    void select_src(QString src);       // triggered when the user select-input-src
    void capture_image(int, QString, int, bool, QString);
    void get_fresh(int);
    void get_videorgb();
    void video_press(int, int);
    void video_release(int, int, int, int, int , int);
    //void get_detail();
private:
	GstElement *m_GstPlayer;
    GstElement *m_GstSrc;
    QQuickItem *m_SelectSrc;
    QQuickItem *m_Capturebtn;
    QQuickItem *m_Freshbtn;
    QQuickItem *m_videorgb;
    //QQuickItem *m_Detailbtn;
    QQuickWindow *m_RootObject;
friend class SetGstState;
};

#endif // INTERACTIONCONTROLLER_H
