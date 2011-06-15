/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * https://sourceforge.net/projects/razor-qt/
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.ru>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * END_COMMON_COPYRIGHT_HEADER */


//#include <QStyleOptionToolBar>

#include "razorpanelplugin.h"
#include "razorpanelplugin_p.h"
#include "razorpanellayout.h"
#include <razorqt/razorplugininfo.h>

#include <QStyleOptionToolBar>
#include <QPainter>
#include <QToolTip>
#include <QApplication>
#include <QDebug>
#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QMenu>
#include <razorqt/xdgicon.h>
#include <QMetaEnum>



/************************************************

 ************************************************/
RazorPanelPlugin::RazorPanelPlugin(const RazorPanelPluginStartInfo* startInfo, QWidget* parent):
    QFrame(parent),
    d_ptr(new RazorPanelPluginPrivate(startInfo, this))
{

}


/************************************************

 ************************************************/
RazorPanelPluginPrivate::RazorPanelPluginPrivate(const RazorPanelPluginStartInfo* startInfo, RazorPanelPlugin* parent):
    QObject(parent),
    q_ptr(parent),
    mSettings(new QSettings(startInfo->configFile, QSettings::NativeFormat, this)),
    mConfigId(startInfo->configSection),
    mAlignmentCached(false),
    mMovable(false),
    mPanel(startInfo->panel)
{
    mSettings->beginGroup(startInfo->configSection);

    Q_Q(RazorPanelPlugin);
    q->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    q->setWindowTitle(startInfo->pluginInfo->name());
    mLayout = new QBoxLayout(mPanel->isHorizontal() ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom, q);
    mLayout->setSpacing(0);
    mLayout->setMargin(0);
    mLayout->setContentsMargins(0, 0, 0, 0);

}


/************************************************

 ************************************************/
RazorPanelPlugin::~RazorPanelPlugin()
{
}


/************************************************

 ************************************************/
RazorPanelPluginPrivate::~RazorPanelPluginPrivate()
{
}


/************************************************

 ************************************************/
QSettings& RazorPanelPlugin::settings() const
{
    Q_D(const RazorPanelPlugin);
    return d->settings();
}


/************************************************

 ************************************************/
RazorPanel* RazorPanelPlugin::panel() const
{
    Q_D(const RazorPanelPlugin);
    return d->panel();
}


/************************************************

 ************************************************/
void RazorPanelPlugin::addWidget(QWidget* widget)
{
    Q_D(RazorPanelPlugin);
    d->addWidget(widget);
}


/************************************************

 ************************************************/
void RazorPanelPluginPrivate::addWidget(QWidget* widget)
{
    mLayout->addWidget(widget);
}


/************************************************

 ************************************************/
QBoxLayout* RazorPanelPlugin::layout() const
{
    Q_D(const RazorPanelPlugin);
    return d->layout();
}


/************************************************

 ************************************************/
void RazorPanelPlugin::layoutDirectionChanged(QBoxLayout::Direction direction)
{
    Q_D(RazorPanelPlugin);
    d->layoutDirectionChanged(direction);
}


/************************************************

 ************************************************/
void RazorPanelPluginPrivate::layoutDirectionChanged(QBoxLayout::Direction direction)
{
    mLayout->setDirection(direction);
}


/************************************************

 ************************************************/
void RazorPanelPlugin::updateStyleSheet()
{
    Q_D(RazorPanelPlugin);
    d->updateStyleSheet();
}


/************************************************

 ************************************************/
void RazorPanelPluginPrivate::updateStyleSheet()
{
    Q_Q(RazorPanelPlugin);
    q->style()->unpolish(q);
    q->style()->polish(q);
}


/************************************************

 ************************************************/
void RazorPanelPluginPrivate::initStyleOption(QStyleOptionToolBar *option) const
{
    Q_Q(const RazorPanelPlugin);
    option->initFrom(q);
    option->state |= QStyle::State_Horizontal;
    if (mMovable)
        option->features = QStyleOptionToolBar::Movable;
}


/************************************************

 ************************************************/
QRect RazorPanelPluginPrivate::handleRect()
{
    Q_Q(RazorPanelPlugin);
    QStyleOptionToolBar opt;
    initStyleOption(&opt);

    return q->style()->subElementRect(QStyle::SE_ToolBarHandle, &opt, q);
}


/************************************************

 ************************************************/
bool RazorPanelPlugin::isMovable() const
{
    Q_D(const RazorPanelPlugin);
    return d->isMovable();
}


/************************************************

 ************************************************/
void RazorPanelPlugin::setMovable(bool movable)
{
    Q_D(RazorPanelPlugin);
    d->setMovable(movable);
    emit movableChanged(d->isMovable());
}


/************************************************

 ************************************************/
void RazorPanelPluginPrivate::setMovable(bool movable)
{
    if (mMovable == movable)
        return;

    Q_Q(RazorPanelPlugin);
    mMovable = movable;

    QMargins m = q->contentsMargins();

    if (movable)
        m.setLeft(m.left() + handleRect().width());
    else
        m.setLeft(m.left() - handleRect().width());

    q->setContentsMargins(m);

    updateStyleSheet();
}


/************************************************

 ************************************************/
void RazorPanelPlugin::x11EventFilter(XEvent* event)
{
    Q_UNUSED(event)
}


/************************************************

 ************************************************/
void RazorPanelPlugin::paintEvent(QPaintEvent* event)
{
    Q_D(RazorPanelPlugin);
    d->paintEvent(event);
}


/************************************************

 ************************************************/
void RazorPanelPluginPrivate::paintEvent(QPaintEvent* event)
{
    Q_Q(RazorPanelPlugin);

    if (mMovable)
    {
        QPainter p(q);
        QStyle *style = q->style();
        QStyleOptionToolBar opt;
        initStyleOption(&opt);

        opt.rect = style->subElementRect(QStyle::SE_ToolBarHandle, &opt, q);
        if (opt.rect.isValid())
            style->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &p, q);
    }
}


/************************************************

 ************************************************/
RazorPanelPlugin::Alignment RazorPanelPlugin::alignment() const
{
    Q_D(const RazorPanelPlugin);
    return d->alignment();
}


/************************************************

 ************************************************/
RazorPanelPlugin::Alignment RazorPanelPluginPrivate::alignment() const
{
    if (!mAlignmentCached)
    {
        Q_Q(const RazorPanelPlugin);
        QString s = settings().value("alignment").toString();

        // Retrun default value
        if (s.isEmpty())
            mAlignment =  q->preferredAlignment();
        else
            mAlignment = (s.toUpper() == "RIGHT") ?
                         RazorPanelPlugin::AlignRight :
                         RazorPanelPlugin::AlignLeft;

        mAlignmentCached = true;
    }

    return mAlignment;
}


/************************************************

 ************************************************/
void RazorPanelPlugin::setAlignment(Alignment alignment)
{
    Q_D(RazorPanelPlugin);
    d->setAlignment(alignment);
    emit alignmentChanged();
}


/************************************************

 ************************************************/
void RazorPanelPluginPrivate::setAlignment(RazorPanelPlugin::Alignment alignment)
{
    mAlignment = alignment;
    updateStyleSheet();
}


/************************************************

 ************************************************/
QString RazorPanelPlugin::configId() const
{
    Q_D(const RazorPanelPlugin);
    return d->configId();
}


