#include "qlabellvgl.h"

#include <QDebug>
#include <QMouseEvent>

extern "C" int app_main(const int argc, const char **argv);

QLabelLvgl::QLabelLvgl(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent)
    , display_image(LV_HOR_RES_MAX, LV_VER_RES_MAX, QImage::Format_RGB16) {
    Q_UNUSED(f);

    parent->setGeometry(0,0, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    this->setMaximumWidth(LV_HOR_RES_MAX);
    this->setMaximumHeight(LV_VER_RES_MAX);

    last_state = LV_INDEV_STATE_REL;

    installEventFilter(this);

    // init lvgl
    lvglInit();

    // start lvgl tick
    timerId = startTimer(LVGL_TICK_TIME);

    startLVGLWorkThread();
}

QLabelLvgl::~QLabelLvgl()
{
    t->quit();
    t->wait();

    killTimer(timerId);
}

void QLabelLvgl::do_disp_flush(const lv_area_t *area, lv_color_t *color_p, bool last)
{
    int32_t x, y;
    lv_color_t pixel;
    QRgb pixel_output;

    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            pixel = *color_p;
            pixel_output = pixel.ch.red << (16 + 3);
            pixel_output |= pixel.ch.green << (8 + 2);
            pixel_output |= pixel.ch.blue << 3;

            display_image.setPixelColor(x,y, pixel_output);
            color_p++;
        }
    }
    if (last) {
        this->setPixmap(QPixmap::fromImage(display_image));
    }
}

void QLabelLvgl::mousePressEvent(QMouseEvent* event) {
    mousePressed(event->x(), event->y());
    qDebug() << "mousePressEvent";
    last_state = LV_INDEV_STATE_PR;
}

void QLabelLvgl::mouseReleaseEvent(QMouseEvent* event) {
    mouseReleased(event->x(), event->y());
    qDebug() << "mouseReleaseEvent";
    last_state = LV_INDEV_STATE_REL;
}

void QLabelLvgl::timerEvent(QTimerEvent *event)
{
    static int cnt = 0;
    if (event->timerId() == timerId) {
        lv_tick_inc(LVGL_TICK_TIME);
        if (cnt++ > 4) {
            cnt = 0;
            lv_task_handler();
        }
    }
}

void QLabelLvgl::lvglInit()
{
    lv_init();
    lvglDispInit();
}

void QLabelLvgl::lvglDispInit()
{
    // init draw_buf
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, DISP_BUF_SIZE);

    // init drv
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;

    // flush cb
    disp_drv.flush_cb = disp_flush;

    disp_drv.draw_buf = &draw_buf_dsc_1;

    disp_drv.user_data = (void *)this;

    lv_disp_drv_register(&disp_drv);
}

bool QLabelLvgl::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        int x = mouseEvent->pos().x();
        int y = mouseEvent->pos().y();
        qDebug("Mouse move %d, %d\n", x, y);
    }
    return false;
}

void QLabelLvgl::disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    QLabelLvgl *qlabel = (QLabelLvgl *)disp->user_data;

    bool last = lv_disp_flush_is_last(disp);

    qlabel->do_disp_flush(area, color_p, last);

    lv_disp_flush_ready(disp);
}

void QLabelLvgl::startLVGLWorkThread()
{
    t = QThread::create([]{
        int ret = app_main(0, nullptr);
        return ret;
    });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
}
