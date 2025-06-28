#include "radar-widget.h"
#include <QPainter>
#include <QDateTime>
#include <QMouseEvent>
#include <cmath>
#include <random>

RadarWidget::RadarWidget(QWidget* parent)
    : QOpenGLWidget(parent), QOpenGLFunctions()
{

    QSurfaceFormat fmt = format();
    fmt.setStencilBufferSize(8);
    setFormat(fmt);

    setMouseTracking(true);
    _noise_data.resize(_noise_w * _noise_h);

    connect(&_update_timer, &QTimer::timeout, this, [this]() {
        generateNoise();

        makeCurrent();
        glBindTexture(GL_TEXTURE_2D, _noise_tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0,
            _noise_w, _noise_h,
            GL_LUMINANCE, GL_UNSIGNED_BYTE,
            _noise_data.data()
        );
        glBindTexture(GL_TEXTURE_2D, 0);
        doneCurrent();

        _blink_only = false;
        if (_selected_target_id >= 0 && !_blink_timer.isActive()) {
            _blink_on = true;
            _blink_timer.start(200);
        }
        update();
    });
    _update_timer.start(500);
    connect(&_blink_timer, &QTimer::timeout, this, [this]() {
        _blink_only = true;
        _blink_on = !_blink_on;
        update();
    });

    QTimer::singleShot(0, this, SLOT(update()));
}


RadarWidget::~RadarWidget() {
    makeCurrent();
    if (_noise_tex) {
        glDeleteTextures(1, &_noise_tex);
        _noise_tex = 0;
    }
    if (_grid_list) {
        glDeleteLists(_grid_list, 1);
        _grid_list = 0;
    }
    doneCurrent();
}

void RadarWidget::initializeGL() {
    initializeOpenGLFunctions();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &_noise_tex);
    glBindTexture(GL_TEXTURE_2D, _noise_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexImage2D(
        GL_TEXTURE_2D, 0,
        GL_LUMINANCE,
        _noise_w, _noise_h, 0,
        GL_LUMINANCE,
        GL_UNSIGNED_BYTE,
        nullptr
    );
    glBindTexture(GL_TEXTURE_2D, 0);

    _noise_data.resize(_noise_w * _noise_h);

    _grid_list = glGenLists(1);
    glNewList(_grid_list, GL_COMPILE);

    glColor3f(0.5f, 0.5f, 0.5f);
    int maxR = std::min(width(), height()) / 2;
    for (int r_m = 200; r_m < 1000; r_m += 200) {
        double radius = double(r_m) * maxR / 1000.0;
        glBegin(GL_LINE_LOOP);
        for (int deg = 0; deg < 360; deg += 5) {
            double a = deg * EIGEN_PI / 180.0;
            glVertex2f(width() / 2 + radius * cos(a),
                height() / 2 + radius * sin(a));
        }
        glEnd();
    }

    glColor3f(0.7f, 0.7f, 0.7f);
    glLineStipple(1, 0x00FF);
    glEnable(GL_LINE_STIPPLE);
    for (int i = 0; i < 8; ++i) {
        double a = i * EIGEN_PI / 4.0;
        double R = maxR;
        glBegin(GL_LINES);
        glVertex2f(width() / 2, height() / 2);
        glVertex2f(width() / 2 + R * cos(a),
            height() / 2 + R * sin(a));
        glEnd();
    }
    glDisable(GL_LINE_STIPPLE);

    glEndList();

    
}

void RadarWidget::onUpdateTimer() {
    generateNoise();

    glBindTexture(GL_TEXTURE_2D, _noise_tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
        _noise_w, _noise_h,
        GL_LUMINANCE, GL_UNSIGNED_BYTE,
        _noise_data.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    _blink_only = false;
    if (_selected_target_id >= 0 && !_blink_timer.isActive()) {
        _blink_on = true;
        _blink_timer.start(200);
    }

    update();
}

void RadarWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);

    if (_grid_list) {
        glDeleteLists(_grid_list, 1);
        _grid_list = 0;
    }

    _grid_list = glGenLists(1);
    glNewList(_grid_list, GL_COMPILE);

    glColor3f(0.5f, 0.5f, 0.5f);
    int maxR = std::min(w, h) / 2;
    for (int r_m = 200; r_m <= 1000; r_m += 200) {
        double radius = double(r_m) * maxR / 1000.0;
        glBegin(GL_LINE_LOOP);
        for (int deg = 0; deg < 360; deg += 5) {
            double a = deg * EIGEN_PI / 180.0;
            glVertex2f(w / 2 + radius * cos(a),
                h / 2 + radius * sin(a));
        }
        glEnd();
    }

    glColor3f(0.7f, 0.7f, 0.7f);
    glLineStipple(1, 0x00FF);
    glEnable(GL_LINE_STIPPLE);
    for (int i = 0; i < 8; ++i) {
        double a = i * EIGEN_PI / 4.0;
        double R = maxR;
        glBegin(GL_LINES);
        glVertex2f(w / 2, h / 2);
        glVertex2f(w / 2 + R * cos(a),
            h / 2 + R * sin(a));
        glEnd();
    }
    glDisable(GL_LINE_STIPPLE);

    glEndList();
}

void RadarWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width(), height(), 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float cx = width() * 0.5f;
    float cy = height() * 0.5f;
    float maxR = std::min(width(), height()) * 0.5f;
    const int segs = 64;

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segs; ++i) {
        float a = 2 * EIGEN_PI * i / segs;
        glVertex2f(cx + cos(a) * maxR, cy + sin(a) * maxR);
    }
    glEnd();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    drawNoise();

    glDisable(GL_STENCIL_TEST);

    glCallList(_grid_list);
    drawTrails();
    drawTargets();

    if (_selected_target_id >= 0 && _blink_on) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for (const auto& t : _targets) {
            if (t.id == _selected_target_id) {
                drawSingleTarget(t);
                break;
            }
        }
        glDisable(GL_BLEND);
    }

    glLineWidth(3.0f);
    glColor3f(1.0f, 0.5f, 0.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segs; ++i) {
        float a = 2 * EIGEN_PI * i / segs;
        glVertex2f(cx + cos(a) * maxR, cy + sin(a) * maxR);
    }
    glEnd();
    glLineWidth(1.0f);
    glColor3f(1, 1, 1);
}

void RadarWidget::drawSingleTarget(const Target& target) {
    double size = 10.0;
    QPointF center = polarToPixel(target.distance, target.angle);

    glColor3f(1.0f, 1.0f, 0.0f);

    glBegin(GL_TRIANGLES);
    glVertex2f(center.x() + size * cos(target.direction),
        center.y() + size * sin(target.direction));
    glVertex2f(center.x() + size * cos(target.direction + 2.618),
        center.y() + size * sin(target.direction + 2.618));
    glVertex2f(center.x() + size * cos(target.direction - 2.618),
        center.y() + size * sin(target.direction - 2.618));
    glEnd();
}


void RadarWidget::setTargets(const std::vector<Target>& targets) {
    QMutexLocker lk(&_data_mutex);
    _targets = targets;
}

void RadarWidget::drawTargets() {
    for (const auto& target : _targets) {
        double size = 10.0;
        QPointF center = polarToPixel(target.distance, target.angle);

        if (target.id == _selected_target_id && _blink_on) {
            glColor3f(1.0f, 1.0f, 0.0f);
        }
        else {
            glColor3f(target.color[0], target.color[1], target.color[2]);
        }

        glBegin(GL_TRIANGLES);
        glVertex2f(center.x() + size * cos(target.direction),
            center.y() + size * sin(target.direction));
        glVertex2f(center.x() + size * cos(target.direction + 2.618),
            center.y() + size * sin(target.direction + 2.618));
        glVertex2f(center.x() + size * cos(target.direction - 2.618),
            center.y() + size * sin(target.direction - 2.618));
        glEnd();
    }
}

void RadarWidget::drawTrails() {
    for (const auto& target : _targets) {
        glColor3f(target.color[0], target.color[1], target.color[2]);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        for (const auto& point : target.trail) {
            QPointF p = polarToPixel(point[0], point[1]);
            glVertex2f(p.x(), p.y());
        }
        glEnd();
    }
}

void RadarWidget::generateNoise() {
    static thread_local std::mt19937 gen{ std::random_device{}() };
    static thread_local std::uniform_int_distribution<> dis{ 0, 80 };
    for (auto& b : _noise_data) {
        b = static_cast<uint8_t>(dis(gen));
    }
}

void RadarWidget::drawNoise() {
    glColor3f(1.0f, 1.0f, 1.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _noise_tex);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(width(), 0);
    glTexCoord2f(1, 1); glVertex2f(width(), height());
    glTexCoord2f(0, 1); glVertex2f(0, height());
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

Eigen::Vector2d RadarWidget::pixelToPolar(const QPointF& pos) const {
    double dx = pos.x() - width() / 2.0;
    double dy = height() / 2.0 - pos.y();

    double distance = sqrt(dx * dx + dy * dy);
    double angle = atan2(dy, dx);

    double maxPixels = std::min(width(), height()) / 2.0;
    return { (distance / maxPixels) * 1000.0, angle };
}

QPointF RadarWidget::polarToPixel(double distance, double angle) const {
    double maxPixels = std::min(width(), height()) / 2.0;
    double r = (distance / 1000.0) * maxPixels;
    return QPointF(
        width() / 2 + r * cos(angle),
        height() / 2 + r * sin(angle)
    );
}

void RadarWidget::mouseMoveEvent(QMouseEvent* event) {
    _cursor_pos = event->pos();
    Eigen::Vector2d polar = pixelToPolar(_cursor_pos);
    emit cursorPositionChanged(polar[0], polar[1] * 180 / EIGEN_PI);
}

void RadarWidget::mousePressEvent(QMouseEvent* event) {
    Eigen::Vector2d polar = pixelToPolar(event->pos());
    double min_dist = std::numeric_limits<double>::max();
    int selected_id = -1;

    for (const auto& target : _targets) {
        double dx = target.distance - polar[0];
        double dy = target.angle - polar[1];
        double dist = sqrt(dx * dx + dy * dy);

        if (dist < min_dist && dist < 50) {
            min_dist = dist;
            selected_id = target.id;
        }
    }

    _selected_target_id = selected_id;
    emit targetSelected(selected_id);
}

void RadarWidget::selectTarget(int id) {
    _selected_target_id = id;

    if (id < 0) {
        _blink_timer.stop();
        _blink_only = false;
        _blink_on = false;
    }
}