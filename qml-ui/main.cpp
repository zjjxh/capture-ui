#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "serialport.h"
#include <QSharedMemory>

// gstreamer includes
#include <gst/gst.h>

// application includes
#include "gst_bus_poller.h"
#include "interaction_controller.h"

int main(int argc, char *argv[])
{
    int ret;
    InteractionController * interactionController;
    GstBusPoller * srcPipelinePoller;
    qputenv("GST_DEBUG", "3");

    QSharedMemory shared("demo");//随便填个名字就行
    if (shared.attach())
    {
            return 0;
    }
    shared.create(1);

    gst_init(&argc, &argv);

    QGuiApplication app(argc, argv);

    GstElement *pipeline = gst_pipeline_new(nullptr);
    GstElement *src = gst_element_factory_make("v4l2src", nullptr);

    //add capfilter
    GstElement *capsfilter = gst_element_factory_make("capsfilter", nullptr);
    GstCaps *caps = gst_caps_new_simple ("video/x-raw",
           //"format", G_TYPE_STRING, "ARGB",
           "framerate", GST_TYPE_FRACTION, 15, 1,
           "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
           "width", G_TYPE_INT, 800,
           "height", G_TYPE_INT, 600,
           NULL);
    g_object_set(capsfilter, "caps", caps, nullptr);

    // convert to GL texture (as qmlglsink expects, also this force processing the video accelerated by the GPU)
    GstElement *glupload = gst_element_factory_make("glupload", nullptr);
    // convert from original video format NV12 to format accepted by qmlglsink (RGBA)
    GstElement *glcolorconvert = gst_element_factory_make("glcolorconvert", nullptr);
    // QML element sink
    /* the plugin must be loaded before loading the qml file to register the
     * GstGLVideoItem qml item */
    GstElement *sink = gst_element_factory_make("qmlglsink", nullptr);
    g_object_set(sink, "sync", false, nullptr);

    // link the elements in the pipeline
    gst_bin_add_many(GST_BIN(pipeline), src, capsfilter, glupload, glcolorconvert, sink, nullptr);
    gst_element_link_many(src, capsfilter, glupload, glcolorconvert, sink, nullptr);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    QQuickItem *videoItem;
    QQuickWindow *rootObject;

    /* find and set the videoItem on the sink */
    rootObject = static_cast<QQuickWindow *> (engine.rootObjects().first());
    videoItem = rootObject->findChild<QQuickItem *> ("videoItem");
    g_object_set(sink, "widget", videoItem, nullptr);

    GstBus *srcBus = gst_element_get_bus(pipeline);
    srcPipelinePoller = new GstBusPoller(srcBus, "srcPipelinePoller");
    g_object_unref(srcBus);
    interactionController = new InteractionController(rootObject, pipeline, src);/*pipline sinked. no need to unref*/

    ret = app.exec();

    delete srcPipelinePoller;
    delete interactionController;
    gst_deinit ();

    return ret;
}
