/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/**
    @file GLAabb.cpp
*/

#include <GLAabb.hpp>

GLAabb::GLAabb() : valid_(false)
{
}

GLAabb::~GLAabb()
{
}

void GLAabb::set(float min_x,
                 float min_y,
                 float min_z,
                 float max_x,
                 float max_y,
                 float max_z)
{
    min_[0] = min_x;
    min_[1] = min_y;
    min_[2] = min_z;
    max_[0] = max_x;
    max_[1] = max_y;
    max_[2] = max_z;

    setValid();
}

void GLAabb::set(const QVector3D &min, const QVector3D &max)
{
    min_ = min;
    max_ = max;

    setValid();
}

void GLAabb::set(const std::vector<float> &xyz)
{
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;

    size_t n = xyz.size() / 3;
    if (n > 0)
    {
        min_x = max_x = xyz[0];
        min_y = max_y = xyz[1];
        min_z = max_z = xyz[2];

        for (size_t i = 1; i < n; i++)
        {
            if (xyz[3 * i + 0] < min_x)
                min_x = xyz[3 * i + 0];
            else if (xyz[3 * i + 0] > max_x)
                max_x = xyz[3 * i + 0];
            if (xyz[3 * i + 1] < min_y)
                min_y = xyz[3 * i + 1];
            else if (xyz[3 * i + 1] > max_y)
                max_y = xyz[3 * i + 1];
            if (xyz[3 * i + 2] < min_z)
                min_z = xyz[3 * i + 2];
            else if (xyz[3 * i + 2] > max_z)
                max_z = xyz[3 * i + 2];
        }
    }
    else
    {
        min_x = max_x = 0.F;
        min_y = max_y = 0.F;
        min_z = max_z = 0.F;
    }

    set(min_x, min_y, min_z, max_x, max_y, max_z);
}

void GLAabb::extend(const GLAabb &box)
{
    QVector3D min = box.getMin();
    QVector3D max = box.getMax();

    if (isValid())
    {
        if (min[0] < min_[0])
            min_[0] = min[0];
        if (max[0] > max_[0])
            max_[0] = max[0];
        if (min[1] < min_[1])
            min_[1] = min[1];
        if (max[1] > max_[1])
            max_[1] = max[1];
        if (min[2] < min_[2])
            min_[2] = min[2];
        if (max[2] > max_[2])
            max_[2] = max[2];

        setValid();
    }
    else
    {
        set(min, max);
    }
}

void GLAabb::invalidate()
{
    valid_ = false;
}

void GLAabb::setValid()
{
    QVector3D halfDiagonal = (max_ - min_) * 0.5F;
    center_ = min_ + halfDiagonal;
    radius_ = halfDiagonal.length();

    valid_ = true;
}
