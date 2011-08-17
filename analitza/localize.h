/*************************************************************************************
 *  Copyright (C) 2011 by Aleix Pol <aleixpol@kde.org>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef LOCALIZE_H
#define LOCALIZE_H

/**
 * This file contains the functions that will be used in case kde's i18n is not available.
 */

// #define NO_KDECORE 1 //Just for testing!

#ifndef NO_KDECORE
#  include <klocalizedstring.h>
#else

#include <QString>

QString i18n(const char* a);
template <typename T> QString i18n(const char* a, const T& x1) { return QString(a).arg(x1); }
template <typename T, typename U> QString i18n(const char* a, const T& x1, const U& x2) { return QString(a).arg(x1).arg(x2); }
QString i18nc(const char*, const char* a);
template <typename T> QString i18nc(const char*, const char* a, const T& x1) { return QString(a).arg(x1); }
template <typename T, typename U> QString i18nc(const char*, const char* a, const T& x1, const U& x2) { return QString(a).arg(x1).arg(x2); }

QString i18np(const char* singular, const char* plural, int x1);
template <typename T> QString i18np(const char* singular, const char* plural, int x1, const T& x2)
{ return x1==1 ? QString(singular).arg(x1).arg(x2) : QString(plural).arg(x1).arg(x2); }

#endif

#endif

struct stat;