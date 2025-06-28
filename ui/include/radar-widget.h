#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include <QMutex>
#include <vector>
#include "target.h"

class RadarWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit RadarWidget(QWidget* parent = nullptr);
    ~RadarWidget() override;

    void setTargets(const std::vector<Target>& targets);

signals:
    void cursorPositionChanged(double distance, double angle_deg);
    void targetSelected(int id);

public slots:
    void selectTarget(int id);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void drawTargets();
    void generateNoise();
    void drawNoise();
    void drawTrails();
    void drawSingleTarget(const Target& target);
    
    void onUpdateTimer();
    void onBlinkTimer();

    Eigen::Vector2d pixelToPolar(const QPointF& pos) const;
    QPointF polarToPixel(double distance, double angle) const;

    std::vector<Target> _targets;
    std::vector<uint8_t> _noise_data;
    QTimer _update_timer;
    QTimer _blink_timer;
    QMutex _data_mutex;
    GLuint _grid_list = 0;
    GLuint _noise_tex = 0;
    QPointF _cursor_pos;
    int _noise_w = 1000, _noise_h = 360;
    int _selected_target_id = -1;

    bool _blink_only = false;
    bool _blink_on = false;
};