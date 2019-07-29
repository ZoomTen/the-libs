/****************************************
 *
 *   the-libs - Libraries for the* Apps
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

#include "tswitch.h"
#include <tvariantanimation.h>
#include <QPaintEvent>
#include <QPainter>
#include <QStaticText>

tSwitch::tSwitch(QWidget *parent) : QPushButton(parent)
{
    this->setCheckable(true);
    this->setFixedSize(this->sizeHint());

    innerRect = QRect(0, 0, this->height(), this->width());

    connect(this, SIGNAL(toggled(bool)), this, SLOT(checkChanging(bool)));
}

QPalette::ColorGroup tSwitch::IsActiveColorRole() {
    if (this->isEnabled()) {
        return QPalette::Active;
    } else {
        return QPalette::Disabled;
    }
}

void tSwitch::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    painter.setFont(this->font());
    QFontMetrics metrics(this->font());

    painter.setBrush(this->palette().brush(IsActiveColorRole(), QPalette::Highlight));
    painter.setPen(this->palette().color(IsActiveColorRole(), QPalette::WindowText));
    painter.drawRect(0, 0, innerRect.left(), this->height());

    painter.setBrush(this->palette().brush(IsActiveColorRole(), QPalette::HighlightedText));
    painter.drawRect(innerRect);

    if (iText == "") {
        painter.drawPixmap(innerRect.left() - 10 - this->height() / 2, (this->height() / 2 - 16 / 2), 16, 16, this->iIcon.pixmap(16, 16));
    } else {
        painter.drawStaticText(innerRect.left() - metrics.width(iText) - this->height() / 2, (this->height() / 2 - metrics.height() / 2), QStaticText(iText));
    }
    painter.setBrush(this->palette().brush(IsActiveColorRole(), QPalette::WindowText));
    painter.drawStaticText(innerRect.right() + this->height() / 2, (this->height() / 2 - metrics.height() / 2), QStaticText(oText));

    painter.setPen(this->palette().color(IsActiveColorRole(), QPalette::WindowText));
    painter.setBrush(QBrush(Qt::transparent));
    painter.drawRect(0, 0, this->width() - 1, this->height() - 1);

    event->accept();
}

void tSwitch::mousePressEvent(QMouseEvent *event) {
    mouseClickPoint = event->localPos().toPoint().x();
    initialPoint = mouseClickPoint;
}

void tSwitch::mouseMoveEvent(QMouseEvent *event) {
    if (event->localPos().toPoint().x() < mouseClickPoint) {
        mouseMovedLeft = true;
    } else {
        mouseMovedLeft = false;
    }

    innerRect.translate(event->localPos().toPoint().x() - mouseClickPoint, 0);
    if (innerRect.x() <= 0) {
        innerRect.moveTo(0, 0);
    } else if (innerRect.right() >= this->width()) {
        innerRect.moveTo(this->width() - innerRect.width(), 0);
    }

    mouseClickPoint = event->localPos().toPoint().x();
    this->repaint();
}

void tSwitch::mouseReleaseEvent(QMouseEvent *event) {
    if (initialPoint - 2 < mouseClickPoint && initialPoint + 2 > mouseClickPoint) {
        bool checked = !this->isChecked();
        this->setChecked(checked);
        this->checkChanging(checked);
    } else {
        if (mouseMovedLeft) {
            this->setChecked(false);
            this->checkChanging(false);
        } else {
            this->setChecked(true);
            this->checkChanging(true);
        }
    }
}

void tSwitch::checkChanging(bool checked) {
    tVariantAnimation* animation = new tVariantAnimation();
    animation->setStartValue(innerRect);
    animation->setDuration(250);
    //animation->setForceAnimation(true);

    if (checked) {
        animation->setEndValue(QRect(this->width() - this->height(), 0, this->height(), this->height()));
    } else {
        animation->setEndValue(QRect(0, 0, this->height(), this->height()));
    }

    animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(animation, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        innerRect = value.toRect();
        this->repaint();
    });
    connect(animation, &tVariantAnimation::finished, this, [=] {
        innerRect = animation->endValue().toRect();
        this->repaint();
    });
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    animation->start();
}

QSize tSwitch::sizeHint() const {
    QFontMetrics metrics(this->font());
    int width = 33 * theLibsGlobal::getDPIScaling();
    if (iText == "") {
        width += 8 * theLibsGlobal::getDPIScaling();
    } else {
        width += metrics.width(iText);
    }

    width += metrics.width(oText);
    return QSize(width, 22 * theLibsGlobal::getDPIScaling());
}

QString tSwitch::OnText() {
    return iText;
}

QString tSwitch::OffText() {
    return oText;
}

void tSwitch::setOnText(QString text) {
    iText = text;
    this->iIcon = QIcon();
    this->setFixedSize(this->sizeHint());
    this->repaint();
}

void tSwitch::setOffText(QString text) {
    oText = text;
    this->setFixedSize(this->sizeHint());
    this->repaint();
}

void tSwitch::setOnIcon(QIcon icon) {
    this->iIcon = icon;
    iText = "";
    this->setFixedSize(this->sizeHint());
    this->repaint();
}

QIcon tSwitch::OnIcon() {
    return this->iIcon;
}
