#pragma once

#include <vector>
#include <Eigen/Dense>

struct Target {
    int id;
    double distance;
    double angle;
    double direction;
    Eigen::Vector3d color;
    std::vector<Eigen::Vector2d> trail;

    Target(int id, double dist, double ang, double dir, const Eigen::Vector3d& col);
    Target(int id, double dist, double ang, double dir, const Eigen::Vector3d& col, const std::vector<Eigen::Vector2d>& trl);

    void move();

    Eigen::Vector2d position() const;
};

Eigen::Vector3d generateRandomColor();