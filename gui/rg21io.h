// -*- c-basic-offset: 4 -*-

/*
    Rosegarden-4
    A sequencer and musical notation editor.

    This program is Copyright 2000-2002
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <bownie@bownie.com>

    The moral right of the authors to claim authorship of this work
    has been asserted.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RG21IO_H_
#define _RG21IO_H_

#include <qstringlist.h>
#include <qfile.h>
#include <qtextstream.h>
#include <vector>

#include "progressreporter.h"
#include "NotationTypes.h"

class Rosegarden::Studio;
class Rosegarden::Composition;
class Rosegarden::Segment;

/**
 * Rosegarden 2.1 file import
 */
class RG21Loader : public ProgressReporter
{
public:
    RG21Loader(Rosegarden::Studio *,
	       QObject *parent = 0, const char *name = 0);
    ~RG21Loader();
    
    /**
     * Load and parse the RG2.1 file \a fileName, and write it into the
     * given Composition (clearing the existing segment data first).
     * Return true for success.
     */
    bool load(const QString& fileName, Rosegarden::Composition &);

protected:

    // RG21 note mods
    enum { ModSharp   = (1<<0),
           ModFlat    = (1<<1),
           ModNatural = (1<<2)
    };

    // RG21 chord mods
    enum { ModDot     = (1<<0),
	   ModLegato  = (1<<1),
	   ModAccent  = (1<<2),
	   ModSfz     = (1<<3),
	   ModRfz     = (1<<4),
	   ModTrill   = (1<<5),
	   ModTurn    = (1<<6),
	   ModPause   = (1<<7)
    };

    // RG21 text positions
    enum { TextAboveStave = 0,
	   TextAboveStaveLarge,
	   TextAboveBarLine,
	   TextBelowStave,
	   TextBelowStaveItalic,
	   TextChordName,
	   TextDynamic
    };

    bool parseClef();
    bool parseKey();
    bool parseMetronome();
    bool parseChordItem();
    bool parseRest();
    bool parseText();
    bool parseGroupStart();
    bool parseIndicationStart();
    bool parseBarType();
    bool parseStaveType();

    void closeGroup();
    void closeIndication();
    void closeSegment();

    void setGroupProperties(Rosegarden::Event *);

    long convertRG21Pitch(long rg21pitch, int noteModifier);
    Rosegarden::timeT convertRG21Duration(QStringList::Iterator&);
    std::vector<std::string> convertRG21ChordMods(int chordMod);

    bool readNextLine();

    //--------------- Data members ---------------------------------

    QTextStream *m_stream;

    Rosegarden::Studio *m_studio;
    Rosegarden::Composition* m_composition;
    Rosegarden::Segment* m_currentSegment;
    unsigned int m_currentSegmentTime;
    unsigned int m_currentSegmentNb;
    Rosegarden::Clef m_currentClef;
    Rosegarden::Key m_currentKey;
    Rosegarden::InstrumentId m_currentInstrumentId;

    typedef std::map<int, Rosegarden::Event *> EventIdMap;
    EventIdMap m_indicationsExtant;

    bool m_inGroup;
    long m_groupId;
    std::string m_groupType;
    Rosegarden::timeT m_groupStartTime;
    int m_groupTupledLength;
    int m_groupTupledCount;
    int m_groupUntupledLength;
    int m_groupUntupledCount;

    int m_tieStatus; // 0 -> none, 1 -> tie started, 2 -> seen one note

    QString m_currentLine;
    QString m_currentStaffName;

    QStringList m_tokens;

    unsigned int m_nbStaves;
};


#endif /* _MUSIC_IO_ */
