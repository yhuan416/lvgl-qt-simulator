#ifndef QLABELLVGL_H
#define QLABELLVGL_H

#include <QTimer>
#include <QLabel>
#include <QWidget>
#include <Qt>

#include <QThread>

#include "lv_conf.h"
#include "lvgl.h"

#define DISP_BUF_SIZE (LV_HOR_RES_MAX * 10)

class QLabelLvgl : public QLabel
{
    Q_OBJECT
public:
    explicit QLabelLvgl(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~QLabelLvgl();

    void do_disp_flush(const lv_area_t *area, lv_color_t *color_p, bool last);

Q_SIGNALS:
    void clicked();
    void mousePressed(int x, int y);
    void mouseReleased(int x, int y);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

public Q_SLOTS:
    void timerEvent(QTimerEvent *event);
    void threadExit();

private:
    void lvglInit();
    void lvglDispInit();
    virtual bool eventFilter(QObject *obj, QEvent *event);
    static void disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p);

    void startLVGLWorkThread();

    // draw_buf
    lv_disp_draw_buf_t draw_buf_dsc_1;
    lv_color_t buf_1[DISP_BUF_SIZE];

    // drv
    lv_disp_drv_t disp_drv;

    QImage display_image;

    int timerId;
    int last_state;

    QThread *t;
};

#endif // QLABELLVGL_H
