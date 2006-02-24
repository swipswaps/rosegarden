// -*- c-basic-offset: 4 -*-

/*
    Rosegarden-4
    A sequencer and musical notation editor.

    This program is Copyright 2000-2006
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

#include "notestyle.h"
#include "notefont.h"

#include <qfileinfo.h>
#include <qdir.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>

#include "rosestrings.h"
#include "rosedebug.h"
#include "notationproperties.h"
#include "notationstrings.h"


using Rosegarden::Accidental;
using Rosegarden::Mark;
using Rosegarden::Clef;
using Rosegarden::Note;
using Rosegarden::String;


class NoteStyleFileReader : public QXmlDefaultHandler
{
public:
    NoteStyleFileReader(NoteStyleName name);

    typedef Rosegarden::Exception StyleFileReadFailed;
    
    NoteStyle *getStyle() { return m_style; }

    // Xml handler methods:

    virtual bool startElement
    (const QString& namespaceURI, const QString& localName,
     const QString& qName, const QXmlAttributes& atts);
    
private:
    bool setFromAttributes(Note::Type type, const QXmlAttributes &attributes);

    QString m_errorString;
    NoteStyle *m_style;
    bool m_haveNote;
};


const NoteStyleName NoteStyleFactory::DefaultStyle = "Classical";

std::vector<NoteStyleName>
NoteStyleFactory::getAvailableStyleNames()
{
    std::vector<NoteStyleName> names;

    QString styleDir = KGlobal::dirs()->findResource("appdata", "styles/");
    QDir dir(styleDir);
    if (!dir.exists()) {
        std::cerr << "NoteStyle::getAvailableStyleNames: directory \"" << styleDir
             << "\" not found" << std::endl;
        return names;
    }

    dir.setFilter(QDir::Files | QDir::Readable);
    QStringList files = dir.entryList();
    bool foundDefault = false;

    for (QStringList::Iterator i = files.begin(); i != files.end(); ++i) {
	if ((*i).length() > 4 && (*i).right(4) == ".xml") {
	    QFileInfo fileInfo(QString("%1/%2").arg(styleDir).arg(*i));
	    if (fileInfo.exists() && fileInfo.isReadable()) {
		std::string styleName = qstrtostr((*i).left((*i).length()-4));
		if (styleName == DefaultStyle) foundDefault = true;
		names.push_back(styleName);
	    }
	}
    }

    if (!foundDefault) {
	std::cerr << "NoteStyleFactory::getAvailableStyleNames: WARNING: Default style name \"" << DefaultStyle << "\" not found" << std::endl;
    }

    return names;
}

NoteStyle *
NoteStyleFactory::getStyle(NoteStyleName name)
{
    StyleMap::iterator i = m_styles.find(name);

    if (i == m_styles.end()) {

	try {
	    NoteStyle *newStyle = NoteStyleFileReader(name).getStyle();
	    m_styles[name] = newStyle;
	    return newStyle;

	} catch (NoteStyleFileReader::StyleFileReadFailed f) {
	    std::cerr
		<< "NoteStyleFactory::getStyle: Style file read failed: "
		<< f.getMessage() << std::endl;
	    throw StyleUnavailable("Style file read failed: " +
				   f.getMessage());
	}

    } else {
	return i->second;
    }
}

NoteStyle *
NoteStyleFactory::getStyleForEvent(Rosegarden::Event *event)
{
    NoteStyleName styleName;
    if (event->get<String>(NotationProperties::NOTE_STYLE, styleName)) {
	return getStyle(styleName);
    } else {
	return getStyle(DefaultStyle);
    }
}

NoteStyleFactory::StyleMap NoteStyleFactory::m_styles;



NoteStyle::~NoteStyle()
{
    // nothing
}

const NoteStyle::NoteHeadShape NoteStyle::AngledOval = "angled oval";
const NoteStyle::NoteHeadShape NoteStyle::LevelOval = "level oval";
const NoteStyle::NoteHeadShape NoteStyle::Breve = "breve";
const NoteStyle::NoteHeadShape NoteStyle::Cross = "cross";
const NoteStyle::NoteHeadShape NoteStyle::TriangleUp = "triangle up";
const NoteStyle::NoteHeadShape NoteStyle::TriangleDown = "triangle down";
const NoteStyle::NoteHeadShape NoteStyle::Diamond = "diamond";
const NoteStyle::NoteHeadShape NoteStyle::Rectangle = "rectangle";
const NoteStyle::NoteHeadShape NoteStyle::Number = "number";
const NoteStyle::NoteHeadShape NoteStyle::CustomCharName = "custom character";


NoteStyle::NoteHeadShape
NoteStyle::getShape(Note::Type type)
{
    NoteDescriptionMap::iterator i = m_notes.find(type);
    if (i == m_notes.end()) { 
	if (m_baseStyle) return m_baseStyle->getShape(type);
	std::cerr
	    << "WARNING: NoteStyle::getShape: No shape defined for note type "
	    << type << ", defaulting to AngledOval" << std::endl;
	return AngledOval;
    }

    return i->second.shape;
}


bool
NoteStyle::isFilled(Note::Type type)
{
    NoteDescriptionMap::iterator i = m_notes.find(type);
    if (i == m_notes.end()) { 
	if (m_baseStyle) return m_baseStyle->isFilled(type);
	std::cerr 
	    << "WARNING: NoteStyle::isFilled: No definition for note type "
	    << type << ", defaulting to true" << std::endl;
	return true;
    }

    return i->second.filled;
}


bool
NoteStyle::hasStem(Note::Type type)
{
    NoteDescriptionMap::iterator i = m_notes.find(type);
    if (i == m_notes.end()) { 
	if (m_baseStyle) return m_baseStyle->hasStem(type);
	std::cerr
	    << "WARNING: NoteStyle::hasStem: No definition for note type "
	    << type << ", defaulting to true" << std::endl;
	return true;
    }

    return i->second.stem;
}


int
NoteStyle::getFlagCount(Note::Type type)
{
    NoteDescriptionMap::iterator i = m_notes.find(type);
    if (i == m_notes.end()) { 
	if (m_baseStyle) return m_baseStyle->getFlagCount(type);
	std::cerr 
	    << "WARNING: NoteStyle::getFlagCount: No definition for note type "
	    << type << ", defaulting to 0" << std::endl;
	return 0;
    }

    return i->second.flags;
}


int
NoteStyle::getSlashCount(Note::Type type)
{
    NoteDescriptionMap::iterator i = m_notes.find(type);
    if (i == m_notes.end()) { 
	if (m_baseStyle) return m_baseStyle->getSlashCount(type);
	std::cerr 
	    << "WARNING: NoteStyle::getSlashCount: No definition for note type "
	    << type << ", defaulting to 0" << std::endl;
	return 0;
    }

    return i->second.slashes;
}


void
NoteStyle::getStemFixPoints(Note::Type type,
			    HFixPoint &hfix, VFixPoint &vfix)
{
    NoteDescriptionMap::iterator i = m_notes.find(type);
    if (i == m_notes.end()) { 
	if (m_baseStyle) {
	    m_baseStyle->getStemFixPoints(type, hfix, vfix);
	    return;
	}
	std::cerr 
	    << "WARNING: NoteStyle::getStemFixPoints: "
	    << "No definition for note type " << type
	    << ", defaulting to (Normal,Middle)" << std::endl;
	hfix = Normal;
	vfix = Middle;
	return;
    }

    hfix = i->second.hfix;
    vfix = i->second.vfix;
}
 


NoteStyle::CharNameRec
NoteStyle::getNoteHeadCharName(Note::Type type)
{
    NoteDescriptionMap::iterator i = m_notes.find(type);
    if (i == m_notes.end()) { 
	if (m_baseStyle) return m_baseStyle->getNoteHeadCharName(type);
	std::cerr 
	    << "WARNING: NoteStyle::getNoteHeadCharName: No definition for note type "
	    << type << ", defaulting to NOTEHEAD_BLACK" << std::endl;
	return CharNameRec(NoteCharacterNames::NOTEHEAD_BLACK, false);
    }

    const NoteDescription &desc(i->second);
    CharName name = NoteCharacterNames::UNKNOWN;
    bool inverted = false;

    if (desc.shape == AngledOval) {

	name = desc.filled ? NoteCharacterNames::NOTEHEAD_BLACK
	                   : NoteCharacterNames::VOID_NOTEHEAD;

    } else if (desc.shape == LevelOval) {

	if (desc.filled) {
	    std::cerr << "WARNING: NoteStyle::getNoteHeadCharName: No filled level oval head" << std::endl;
	}
	name = NoteCharacterNames::WHOLE_NOTE;
	
    } else if (desc.shape == Breve) {

	if (desc.filled) {
	    std::cerr << "WARNING: NoteStyle::getNoteHeadCharName: No filled breve head" << std::endl;
	}
	name = NoteCharacterNames::BREVE;
	
    } else if (desc.shape == Cross) {

	name = desc.filled ? NoteCharacterNames::X_NOTEHEAD
	                   : NoteCharacterNames::CIRCLE_X_NOTEHEAD;

    } else if (desc.shape == TriangleUp) {

	name = desc.filled ? NoteCharacterNames::TRIANGLE_NOTEHEAD_UP_BLACK
                           : NoteCharacterNames::TRIANGLE_NOTEHEAD_UP_WHITE;

    } else if (desc.shape == TriangleDown) {

	name = desc.filled ? NoteCharacterNames::TRIANGLE_NOTEHEAD_UP_BLACK
                           : NoteCharacterNames::TRIANGLE_NOTEHEAD_UP_WHITE;
	inverted = true;

    } else if (desc.shape == Diamond) {

	name = desc.filled ? NoteCharacterNames::SEMIBREVIS_BLACK
 	                   : NoteCharacterNames::SEMIBREVIS_WHITE;

    } else if (desc.shape == Rectangle) {

	name = desc.filled ? NoteCharacterNames::SQUARE_NOTEHEAD_BLACK
                           : NoteCharacterNames::SQUARE_NOTEHEAD_WHITE;
	
    } else if (desc.shape == Number) {

	std::cerr << "WARNING: NoteStyle::getNoteHeadCharName: Number not yet implemented" << std::endl;
	name = NoteCharacterNames::UNKNOWN; //!!!

    } else if (desc.shape == CustomCharName) {
	
	name = desc.charName;

    } else {

	name = NoteCharacterNames::UNKNOWN;
    }

    return CharNameRec(name, inverted);
}

CharName
NoteStyle::getAccidentalCharName(const Accidental &a)
{
    if      (a == Rosegarden::Accidentals::Sharp)        return NoteCharacterNames::SHARP;
    else if (a == Rosegarden::Accidentals::Flat)         return NoteCharacterNames::FLAT;
    else if (a == Rosegarden::Accidentals::Natural)      return NoteCharacterNames::NATURAL;
    else if (a == Rosegarden::Accidentals::DoubleSharp)  return NoteCharacterNames::DOUBLE_SHARP;
    else if (a == Rosegarden::Accidentals::DoubleFlat)   return NoteCharacterNames::DOUBLE_FLAT;
    return NoteCharacterNames::UNKNOWN;
}

CharName
NoteStyle::getMarkCharName(const Mark &mark)
{
    if      (mark == Rosegarden::Marks::Accent)    return NoteCharacterNames::ACCENT;
    else if (mark == Rosegarden::Marks::Tenuto)    return NoteCharacterNames::TENUTO;
    else if (mark == Rosegarden::Marks::Staccato)  return NoteCharacterNames::STACCATO;
    else if (mark == Rosegarden::Marks::Staccatissimo) return NoteCharacterNames::STACCATISSIMO;
    else if (mark == Rosegarden::Marks::Marcato)   return NoteCharacterNames::MARCATO;
    else if (mark == Rosegarden::Marks::Trill)     return NoteCharacterNames::TRILL;
    else if (mark == Rosegarden::Marks::LongTrill) return NoteCharacterNames::TRILL;
    else if (mark == Rosegarden::Marks::TrillLine) return NoteCharacterNames::TRILL_LINE;
    else if (mark == Rosegarden::Marks::Turn)      return NoteCharacterNames::TURN;
    else if (mark == Rosegarden::Marks::Pause)     return NoteCharacterNames::FERMATA;
    else if (mark == Rosegarden::Marks::UpBow)     return NoteCharacterNames::UP_BOW;
    else if (mark == Rosegarden::Marks::DownBow)   return NoteCharacterNames::DOWN_BOW;
    else if (mark == Rosegarden::Marks::Mordent)
	return NoteCharacterNames::MORDENT;
    else if (mark == Rosegarden::Marks::MordentInverted)
	return NoteCharacterNames::MORDENT_INVERTED;
    else if (mark == Rosegarden::Marks::MordentLong)
	return NoteCharacterNames::MORDENT_LONG;
    else if (mark == Rosegarden::Marks::MordentLongInverted)
	return NoteCharacterNames::MORDENT_LONG_INVERTED;
    // Things like "sf" and "rf" are generated from text fonts
    return NoteCharacterNames::UNKNOWN;
}
 
CharName
NoteStyle::getClefCharName(const Clef &clef)
{
    std::string clefType(clef.getClefType());

    if (clefType == Clef::Bass) {
        return NoteCharacterNames::F_CLEF;
    } else if (clefType == Clef::Treble) {
        return NoteCharacterNames::G_CLEF;
    } else {
        return NoteCharacterNames::C_CLEF;
    }
}

CharName
NoteStyle::getRestCharName(Note::Type type, bool restOutsideStave)
{
    switch (type) {
    case Note::Hemidemisemiquaver:  return NoteCharacterNames::SIXTY_FOURTH_REST;
    case Note::Demisemiquaver:      return NoteCharacterNames::THIRTY_SECOND_REST;
    case Note::Semiquaver:          return NoteCharacterNames::SIXTEENTH_REST;
    case Note::Quaver:              return NoteCharacterNames::EIGHTH_REST;
    case Note::Crotchet:            return NoteCharacterNames::QUARTER_REST;
    case Note::Minim:               return restOutsideStave ?
					NoteCharacterNames::HALF_REST
				      : NoteCharacterNames::HALF_REST_ON_STAFF;
    case Note::Semibreve:           return restOutsideStave ?
    					NoteCharacterNames::WHOLE_REST
				      : NoteCharacterNames::WHOLE_REST_ON_STAFF;
    case Note::Breve:               return restOutsideStave ?
					NoteCharacterNames::MULTI_REST
				      : NoteCharacterNames::MULTI_REST_ON_STAFF;
    default:
        return NoteCharacterNames::UNKNOWN;
    }
}

CharName
NoteStyle::getPartialFlagCharName(bool final)
{
    if (final) return NoteCharacterNames::FLAG_PARTIAL_FINAL;
    else return NoteCharacterNames::FLAG_PARTIAL;
}

CharName
NoteStyle::getFlagCharName(int flagCount)
{
    switch (flagCount) {
    case  1: return NoteCharacterNames::FLAG_1;
    case  2: return NoteCharacterNames::FLAG_2;
    case  3: return NoteCharacterNames::FLAG_3;
    case  4: return NoteCharacterNames::FLAG_4;
    default: return NoteCharacterNames::UNKNOWN;
    }
}

CharName
NoteStyle::getTimeSignatureDigitName(int digit)
{
    switch (digit) {
    case  0: return NoteCharacterNames::DIGIT_ZERO;
    case  1: return NoteCharacterNames::DIGIT_ONE;
    case  2: return NoteCharacterNames::DIGIT_TWO;
    case  3: return NoteCharacterNames::DIGIT_THREE;
    case  4: return NoteCharacterNames::DIGIT_FOUR;
    case  5: return NoteCharacterNames::DIGIT_FIVE;
    case  6: return NoteCharacterNames::DIGIT_SIX;
    case  7: return NoteCharacterNames::DIGIT_SEVEN;
    case  8: return NoteCharacterNames::DIGIT_EIGHT;
    case  9: return NoteCharacterNames::DIGIT_NINE;
    default: return NoteCharacterNames::UNKNOWN;
    }
}

void
NoteStyle::setBaseStyle(NoteStyleName name)
{
    try {
	m_baseStyle = NoteStyleFactory::getStyle(name);
	if (m_baseStyle == this) m_baseStyle = 0;
    } catch (NoteStyleFactory::StyleUnavailable u) {
	if (name != NoteStyleFactory::DefaultStyle) {
	    std::cerr
		<< "NoteStyle::setBaseStyle: Base style "
		<< name << " not available, defaulting to "
		<< NoteStyleFactory::DefaultStyle << std::endl;
	    setBaseStyle(NoteStyleFactory::DefaultStyle);
	} else {
	    std::cerr
		<< "NoteStyle::setBaseStyle: Base style "
		<< name << " not available" << std::endl;
	    m_baseStyle = 0;
	}
    }	    
}

void
NoteStyle::checkDescription(Note::Type note)
{
    if (m_baseStyle && (m_notes.find(note) == m_notes.end())) {
	m_baseStyle->checkDescription(note);
	m_notes[note] = m_baseStyle->m_notes[note];
    }
}

void
NoteStyle::setShape(Note::Type note, NoteHeadShape shape)
{
    checkDescription(note);
    m_notes[note].shape = shape;
}

void
NoteStyle::setCharName(Note::Type note, CharName charName)
{
    checkDescription(note);
    m_notes[note].charName = charName;
}

void
NoteStyle::setFilled(Note::Type note, bool filled)
{
    checkDescription(note);
    m_notes[note].filled = filled;
}

void
NoteStyle::setStem(Note::Type note, bool stem)
{
    checkDescription(note);
    m_notes[note].stem = stem;
}

void
NoteStyle::setFlagCount(Note::Type note, int flags)
{
    checkDescription(note);
    m_notes[note].flags = flags;
}

void
NoteStyle::setSlashCount(Note::Type note, int slashes)
{
    checkDescription(note);
    m_notes[note].slashes = slashes;
}

void
NoteStyle::setStemFixPoints(Note::Type note, HFixPoint hfix, VFixPoint vfix)
{
    checkDescription(note);
    m_notes[note].hfix = hfix;
    m_notes[note].vfix = vfix;
}    


NoteStyleFileReader::NoteStyleFileReader(std::string name) :
    m_style(new NoteStyle(name)),
    m_haveNote(false)
{
    QString styleDirectory =
	KGlobal::dirs()->findResource("appdata", "styles/");

    QString styleFileName =
	QString("%1/%2.xml").arg(styleDirectory).arg(strtoqstr(name));

    QFileInfo fileInfo(styleFileName);

    if (!fileInfo.isReadable()) {
        throw StyleFileReadFailed
	    (qstrtostr(i18n("Can't open style file %1").arg(styleFileName)));
    }

    QFile styleFile(styleFileName);

    QXmlInputSource source(styleFile);
    QXmlSimpleReader reader;
    reader.setContentHandler(this);
    reader.setErrorHandler(this);
    bool ok = reader.parse(source);
    styleFile.close();

    if (!ok) {
	throw StyleFileReadFailed(qstrtostr(m_errorString));
    }
}

bool
NoteStyleFileReader::startElement(const QString &, const QString &,
				  const QString &qName,
				  const QXmlAttributes &attributes)
{
    QString lcName = qName.lower();

    if (lcName == "rosegarden-note-style") {

	QString s = attributes.value("base-style");
	if (s) m_style->setBaseStyle(qstrtostr(s));

    } else if (lcName == "note") {

	m_haveNote = true;
	
	QString s = attributes.value("type");
	if (!s) {
	    m_errorString = i18n("type is a required attribute of note");
	    return false;
	}
	
	try {
	    Note::Type type = NotationStrings::getNoteForName(s).getNoteType();
	    if (!setFromAttributes(type, attributes)) return false;

	} catch (NotationStrings::MalformedNoteName n) {
	    m_errorString = i18n("Unrecognised note name %1").arg(s);
	    return false;
	}

    } else if (lcName == "global") {

	if (m_haveNote) {
	    m_errorString = i18n("global element must precede note elements");
	    return false;
	}
	    
	for (Note::Type type = Note::Shortest; type <= Note::Longest; ++type) {
	    if (!setFromAttributes(type, attributes)) return false;
	}
    }

    return true;
}


bool
NoteStyleFileReader::setFromAttributes(Note::Type type,
				       const QXmlAttributes &attributes)
{
    QString s;
    bool haveShape = false;

    s = attributes.value("shape");
    if (s) {
	m_style->setShape(type, qstrtostr(s.lower()));
	haveShape = true;
    }
    
    s = attributes.value("charname");
    if (s) {
	if (haveShape) {
	    m_errorString = i18n("global and note elements may have shape "
				 "or charname attribute, but not both");
	    return false;
	}
	m_style->setShape(type, NoteStyle::CustomCharName);
	m_style->setCharName(type, qstrtostr(s));
    }

    s = attributes.value("filled");
    if (s) m_style->setFilled(type, s.lower() == "true");
    
    s = attributes.value("stem");
    if (s) m_style->setStem(type, s.lower() == "true");
    
    s = attributes.value("flags");
    if (s) m_style->setFlagCount(type, s.toInt());
    
    s = attributes.value("slashes");
    if (s) m_style->setSlashCount(type, s.toInt());

    NoteStyle::HFixPoint hfix;
    NoteStyle::VFixPoint vfix;
    m_style->getStemFixPoints(type, hfix, vfix);
    bool haveHFix = false;
    bool haveVFix = false;

    s = attributes.value("hfixpoint");
    if (s) {
	s = s.lower();
	haveHFix = true;
	if (s == "normal") hfix = NoteStyle::Normal;
	else if (s == "central") hfix = NoteStyle::Central;
	else if (s == "reversed") hfix = NoteStyle::Reversed;
	else haveHFix = false;
    }

    s = attributes.value("vfixpoint");
    if (s) {
	s = s.lower();
	haveVFix = true;
	if (s == "near") vfix = NoteStyle::Near;
	else if (s == "middle") vfix = NoteStyle::Middle;
	else if (s == "far") vfix = NoteStyle::Far;
	else haveVFix = false;
    }

    if (haveHFix || haveVFix) {
	m_style->setStemFixPoints(type, hfix, vfix);
	// otherwise they inherit from base style, so avoid setting here
    }

    return true;
}


