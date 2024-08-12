// Header-only utility functions to convert tracker coordinates to human coordinates

#include <iostream>
#include <cmath>
#include <array>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

using Eigen::Quaterniond;
using Eigen::Vector3d;
using Eigen::Vector4d;


Vector3d tracker_xyz_to_human_xyz(const Vector3d& tracker_xyz) {
    Vector3d human_xyz;
    human_xyz << -tracker_xyz[0], -tracker_xyz[1], tracker_xyz[2];
    return human_xyz;
}

Quaterniond tracker_quat_to_human_rotation(const Vector4d& tracker_quat) {
    static bool initialized = false;
    static Vector4d SIGNS;
    static Vector4d Q0;
    static Quaterniond R0_INV;

    if (!initialized) {
        SIGNS << 1., 1., -1., -1.;                    // xyzw
        Q0 << sqrt(1. / 2.), 0., sqrt(1. / 2.), 0.;   // xyzw
        R0_INV = Quaterniond(SIGNS.asDiagonal() * Q0).inverse();
        initialized = true;
    }

    Quaterniond rot = Quaterniond(SIGNS.asDiagonal() * tracker_quat) * R0_INV;

    return rot;
}
