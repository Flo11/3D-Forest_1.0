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
    @file Viewer.cpp
*/

#include <GLWidget.hpp>
#include <QDebug>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>
#include <Viewer.hpp>

Viewer::Viewer(QWidget *parent) : QWidget(parent)
{
    initializeViewer();
}

Viewer::~Viewer()
{
}

void Viewer::initializeViewer()
{
    setViewportLayout(ViewportLayout::VIEWPORT_LAYOUT_SINGLE);
}

GLWidget *Viewer::createViewport()
{
    GLWidget *viewport = new GLWidget(this);
    viewport->setViewer(this);
    viewport->setSelected(false);

    return viewport;
}

void Viewer::setViewportLayout(ViewportLayout viewportLayout)
{
    // Remove the current layout
    QLayout *oldLayout = layout();
    if (oldLayout)
    {
        Q_ASSERT(oldLayout->count() == 1);
        QLayoutItem *item = oldLayout->itemAt(0);
        GLWidget *viewport = dynamic_cast<GLWidget *>(item->widget());
        if (viewport)
        {
            oldLayout->removeWidget(viewport);
        }
        else
        {
            QSplitter *splitter = dynamic_cast<QSplitter *>(item->widget());
            if (splitter)
            {
                // Reparent viewports from splitter and find selected viewport
                size_t selectedViewport = 0;
                for (size_t i = 0; i < viewports_.size(); i++)
                {
                    if (viewports_[i]->isSelected())
                    {
                        selectedViewport = i;
                    }
                    viewports_[i]->setParent(this);
                }

                // Delete layout content
                delete splitter;

                // Delete extra viewports
                GLWidget *tmpViewport = viewports_[0];
                viewports_[0] = viewports_[selectedViewport];
                viewports_[selectedViewport] = tmpViewport;
                for (size_t i = 1; i < viewports_.size(); i++)
                {
                    delete viewports_[i];
                }
                viewports_.resize(1);
            }
            else
            {
                Q_UNREACHABLE();
            }
        }

        delete oldLayout;
    }

    // Create the first viewport
    if (viewports_.size() == 0)
    {
        viewports_.resize(1);
        viewports_[0] = createViewport();
        viewports_[0]->setSelected(true);
    }

    // Set layout
    if (viewportLayout == ViewportLayout::VIEWPORT_LAYOUT_SINGLE)
    {
        // Create new layout
        QHBoxLayout *newLayout = new QHBoxLayout;
        newLayout->setContentsMargins(1, 1, 1, 1);
        newLayout->addWidget(viewports_[0]);
        setLayout(newLayout);
    }
    else if (viewportLayout == ViewportLayout::VIEWPORT_LAYOUT_TWO_COLUMNS)
    {
        // Create the second viewport
        viewports_.resize(2);
        viewports_[1] = createViewport();

        // Create new layout
        QSplitter *splitter = new QSplitter;
        splitter->addWidget(viewports_[0]);
        splitter->addWidget(viewports_[1]);
        int w = width() / 2;
        splitter->setSizes(QList<int>({w, w}));

        QHBoxLayout *newLayout = new QHBoxLayout;
        newLayout->setContentsMargins(1, 1, 1, 1);
        newLayout->addWidget(splitter);
        setLayout(newLayout);
    }
    else
    {
        Q_UNREACHABLE();
    }
}

void Viewer::selectViewport(GLWidget *viewport)
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        if (viewports_[i] == viewport)
        {
            viewports_[i]->setSelected(true);
        }
        else
        {
            viewports_[i]->setSelected(false);
        }
        viewports_[i]->update();
    }
}

void Viewer::update(const std::vector<std::shared_ptr<Node>> &scene)
{
    for (size_t i = 0; i < viewports_.size(); i++)
    {
        viewports_[i]->updateScene(scene);
        viewports_[i]->update();
    }
}
