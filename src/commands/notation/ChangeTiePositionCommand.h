
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.

    This program is Copyright 2000-2008
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <richard.bown@ferventsoftware.com>

    The moral rights of Guillaume Laurent, Chris Cannam, and Richard
    Bown to claim authorship of this work have been asserted.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RG_ADJUSTMENUCHANGETIEPOSITIONCOMMAND_H_
#define _RG_ADJUSTMENUCHANGETIEPOSITIONCOMMAND_H_

#include "document/BasicSelectionCommand.h"
#include <qstring.h>
#include <klocale.h>


namespace Rosegarden
{

class EventSelection;

class ChangeTiePositionCommand : public BasicSelectionCommand
{
public:
    ChangeTiePositionCommand(bool above, EventSelection &selection) :
        BasicSelectionCommand(getGlobalName(above), selection, true),
        m_selection(&selection), m_above(above) { }

    static QString getGlobalName(bool above) {
        return above ? i18n("Tie &Above") : i18n("Tie &Below");
    }

protected:
    virtual void modifySegment();

private:
    EventSelection *m_selection;// only used on 1st execute (cf bruteForceRedo)
    bool m_above;
};



}

#endif