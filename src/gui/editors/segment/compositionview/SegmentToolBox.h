
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2009 the Rosegarden development team.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RG_SEGMENTTOOLBOX_H_
#define _RG_SEGMENTTOOLBOX_H_

#include "gui/general/BaseToolBox.h"
#include "SegmentTool.h"


class QString;


namespace Rosegarden
{

class RosegardenDocument;
class CompositionView;


class SegmentToolBox : public BaseToolBox
{
    Q_OBJECT
public:
    SegmentToolBox(CompositionView* parent, RosegardenDocument*);

    virtual SegmentTool* getTool(QString toolName);
    
protected:
    virtual SegmentTool* createTool(QString toolName);

    //--------------- Data members ---------------------------------

    CompositionView* m_canvas;
    RosegardenDocument* m_doc;
};


}

#endif