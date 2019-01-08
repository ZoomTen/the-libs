/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#ifndef TPOPOVER_H
#define TPOPOVER_H

#include <QObject>

struct tPopoverPrivate;
class tPopover : public QObject
{
        Q_OBJECT
    public:
        explicit tPopover(QWidget* popoverWidget, QObject *parent = nullptr);
        ~tPopover();

        void setPopoverWidth(int width);

    signals:
        void dismissed();

    public slots:
        void show(QWidget* parent);
        void dismiss();

    private:
        tPopoverPrivate* d;

        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // TPOPOVER_H