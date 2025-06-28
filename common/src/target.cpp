#include "target.h"
#include <random>
#include <cmath>

constexpr double MAX_DISTANCE = 1000.0;
constexpr double MOVE_DISTANCE = 20.0;
constexpr double DIRECTION_CHANGE_MAX = EIGEN_PI / 4;
constexpr double BOUNDARY_MARGIN = 50.0;

static constexpr size_t TRAIL_SIZE = 3;

Target::Target(
    int id,
    double dist,
    double ang,
    double dir,
    const Eigen::Vector3d& col
) :
    id(id),
    distance(dist),
    angle(ang),
    direction(dir),
    color(col),
    trail(std::vector<Eigen::Vector2d>{TRAIL_SIZE + 1, {dist,angle}})
{
}

Target::Target(
    int id,
    double dist,
    double ang,
    double dir,
    const Eigen::Vector3d& col,
    const std::vector<Eigen::Vector2d>& trl
) :
    id(id),
    distance(dist),
    angle(ang),
    direction(dir),
    color(col),
    trail(trl)
{
}

void Target::move() {
    static thread_local std::mt19937 gen{ std::random_device{}() };
    static thread_local std::uniform_real_distribution<> turn_dis(-DIRECTION_CHANGE_MAX, DIRECTION_CHANGE_MAX);

    double new_dir = direction + turn_dis(gen);

    new_dir = std::fmod(new_dir + 2 * EIGEN_PI, 2 * EIGEN_PI);

    double x = distance * std::cos(angle);
    double y = distance * std::sin(angle);

    double dx = MOVE_DISTANCE * std::cos(new_dir);
    double dy = MOVE_DISTANCE * std::sin(new_dir);
    double x2 = x + dx;
    double y2 = y + dy;

    double r2 = std::hypot(x2, y2);
    double theta2 = std::atan2(y2, x2);
    if (theta2 < 0) theta2 += 2 * EIGEN_PI;

    if (r2 > MAX_DISTANCE) {
        r2 = 2 * MAX_DISTANCE - r2;
        new_dir = std::atan2(-dy, -dx);
    }
    else if (r2 < 0.0) {
        r2 = -r2;
        theta2 = std::atan2(-y2, -x2);
        if (theta2 < 0) {
            theta2 += 2 * EIGEN_PI;
        }
        new_dir = std::atan2(dy, dx);
    }

    distance = r2;
    angle = theta2;
    direction = std::fmod(new_dir + 2 * EIGEN_PI, 2 * EIGEN_PI);

    for (size_t i = 0; i + 1 < trail.size(); ++i) {
        trail[i] = trail[i + 1];
    }
    trail.back() = { distance, angle };
}

Eigen::Vector2d Target::position() const {
    return { distance * std::cos(angle), distance * std::sin(angle) };
}

Eigen::Vector3d generateRandomColor() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    return { dis(gen), dis(gen), dis(gen) };
}