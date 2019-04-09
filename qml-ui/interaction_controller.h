#ifndef INTERACTIONCONTROLLER_H_
#define INTERACTIONCONTROLLER_H_

//==============================================================================
// Includes
//==============================================================================
#include <QObject>
#include <QQuickItem>
#include <QQuickWindow>

#include "gst/gst.h"

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
private:
	GstElement *m_GstPlayer;
    GstElement *m_GstSrc;
    QQuickItem *m_SelectSrc;
	QQuickWindow *m_RootObject;
};

#endif // INTERACTIONCONTROLLER_H
