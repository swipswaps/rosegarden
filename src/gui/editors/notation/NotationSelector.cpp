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

#include "NotationSelector.h"
#include "NotationElement.h"
#include "NotationProperties.h"
#include "NotationStaff.h"
#include "NotationTool.h"
#include "NotationWidget.h"
#include "NotePixmapFactory.h"
#include "NotationMouseEvent.h"
#include "NotationScene.h"

#include "misc/Debug.h"

#include "base/Event.h"
#include "base/NotationTypes.h"
#include "base/PropertyName.h"
#include "base/Selection.h"
#include "base/ViewElement.h"
#include "base/BaseProperties.h"
#include "base/Profiler.h"

#include "commands/edit/MoveAcrossSegmentsCommand.h"
#include "commands/edit/MoveCommand.h"
#include "commands/edit/TransposeCommand.h"
#include "commands/notation/IncrementDisplacementsCommand.h"

#include "gui/general/GUIPalette.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QRect>
#include <QString>
#include <QTimer>


namespace Rosegarden
{
 
using namespace BaseProperties;

NotationSelector::NotationSelector(NotationWidget *widget) :
    NotationTool("notationselector.rc", "NotationSelector", widget),
    m_selectionRect(0),
    m_updateRect(false),
    m_selectedStaff(0),
    m_clickedElement(0),
    m_selectionToMerge(0),
    m_justSelectedBar(false),
    m_wholeStaffSelectionComplete(false)
{
    connect(m_widget, SIGNAL(usedSelection()),
            this, SLOT(slotHideSelection()));

    connect(this, SIGNAL(editElement(NotationStaff *, NotationElement *, bool)),
            m_widget, SLOT(slotEditElement(NotationStaff *, NotationElement *, bool)));

    createAction("insert", SLOT(slotInsertSelected()));
    createAction("erase", SLOT(slotEraseSelected()));
    createAction("collapse_rests_aggressively", SLOT(slotCollapseRestsHard()));
    createAction("respell_flat", SLOT(slotRespellFlat()));
    createAction("respell_sharp", SLOT(slotRespellSharp()));
    createAction("respell_natural", SLOT(slotRespellNatural()));
    createAction("collapse_notes", SLOT(slotCollapseNotes()));
    createAction("interpret", SLOT(slotInterpret()));
    createAction("move_events_up_staff", SLOT(slotStaffAbove()));
    createAction("move_events_down_staff", SLOT(slotStaffBelow()));
    createAction("make_invisible", SLOT(slotMakeInvisible()));
    createAction("make_visible", SLOT(slotMakeVisible()));


    // (this crashed, and it might be superfluous with ^N anyway, so I'm
    // commenting it out, but leaving it here in case I change my mind about
    // fooling with it.)  (DMM)
    //    new KAction(tr("Normalize Rests"), 0, 0, this,
    //                SLOT(slotCollapseRests()), actionCollection(),
    //                "collapse_rests");

    createMenu();
}

NotationSelector::~NotationSelector()
{
    delete m_selectionToMerge;
}

void
NotationSelector::handleLeftButtonPress(const NotationMouseEvent *e)
{
    if (m_justSelectedBar) {
        handleMouseTripleClick(e);
        m_justSelectedBar = false;
        return ;
    }

    m_wholeStaffSelectionComplete = false;

    delete m_selectionToMerge;
    const EventSelection *selectionToMerge = 0;
    if (e->buttons & Qt::ShiftButton) {
        m_clickedShift = true;
        selectionToMerge = m_scene->getSelection();
    } else {
        m_clickedShift = false;
    }
    m_selectionToMerge =
        (selectionToMerge ? new EventSelection(*selectionToMerge) : 0);

    m_selectedStaff = e->staff;
    m_clickedElement = 0;

    if (e->exact) {
        m_clickedElement = e->element;
        if (m_clickedElement) {
            m_lastDragPitch = -400;
            m_lastDragTime = m_clickedElement->event()->getNotationAbsoluteTime();
        }
    }

    if (!m_selectionRect) {
        m_selectionRect = new QGraphicsRectItem;
        m_scene->addItem(m_selectionRect);
        QColor c = GUIPalette::getColour(GUIPalette::SelectionRectangle);
        m_selectionRect->setPen(QPen(c, 2));
        c.setAlpha(50);
        m_selectionRect->setBrush(c);
    }

    m_selectionOrigin = QPointF(e->sceneX, e->sceneY);
    m_selectionRect->setRect(QRectF(m_selectionOrigin, QSize()));
    m_selectionRect->hide();
    m_updateRect = true;
    m_startedFineDrag = false;
}

void
NotationSelector::handleRightButtonPress(const NotationMouseEvent *e)
{
    // if nothing selected, permit the possibility of selecting
    // something before showing the menu
    std::cerr << "NotationSelector::handleRightButtonPress" << std::endl;

    const EventSelection *sel = m_scene->getSelection();

    if (!sel || sel->getSegmentEvents().empty()) {

        if (e->element) {
            m_clickedElement = e->element;
            m_selectedStaff = e->staff;
            m_scene->setSingleSelectedEvent
                (m_selectedStaff, m_clickedElement, true);
        }

        //!!!??? do we really want this? removing for now
//        handleLeftButtonPress(e);
    }

    NotationTool::handleRightButtonPress(e);
}

void NotationSelector::slotClickTimeout()
{
    m_justSelectedBar = false;
}

void NotationSelector::handleMouseDoubleClick(const NotationMouseEvent *e)
{
    NOTATION_DEBUG << "NotationSelector::handleMouseDoubleClick" << endl;

    NotationStaff *staff = e->staff;
    if (!staff) return;
    m_selectedStaff = staff;

    bool advanced = (e->buttons & Qt::ShiftButton);

    if (e->element) {

        emit editElement(staff, e->element, advanced);

    } else {

        QRect rect = staff->getBarExtents(e->sceneX, e->sceneY);

        m_selectionRect->setRect(rect.x() + 0.5, rect.y() + 0.5,
                                 rect.width(), rect.height());
//        m_selectionRect->setY(rect.y());
//        m_selectionRect->setSize(rect.width() - 1, rect.height());

        m_selectionRect->show();
        m_updateRect = false;

        m_justSelectedBar = true;
        QTimer::singleShot(QApplication::doubleClickInterval(), this,
                           SLOT(slotClickTimeout()));
    }

    return;
}

void NotationSelector::handleMouseTripleClick(const NotationMouseEvent *e)
{
    NOTATION_DEBUG << "NotationSelector::handleMouseTripleClick" << endl;
    if (!m_justSelectedBar) return;
    m_justSelectedBar = false;

    NotationStaff *staff = e->staff;
    if (!staff) return;
    m_selectedStaff = staff;

    if (e->element) {

        // should be safe, as we've already set m_justSelectedBar false
        handleLeftButtonPress(e);
        return;

    } else {

        m_selectionRect->setRect(staff->getX(), staff->getY(),
                                 staff->getTotalWidth() - 1,
                                 staff->getTotalHeight() - 1);

        m_selectionRect->show();
        m_updateRect = false;
    }

    m_wholeStaffSelectionComplete = true;

    return;
}

NotationSelector::FollowMode
NotationSelector::handleMouseMove(const NotationMouseEvent *e)
{
    if (!m_updateRect) return NoFollow;

    std::cerr << "NotationSelector::handleMouseMove: staff is " 
              << m_selectedStaff << ", m_updateRect is " << m_updateRect
              << std::endl;

    if (!m_selectedStaff) m_selectedStaff = e->staff;

    int w = int(e->sceneX - m_selectionRect->x());
    int h = int(e->sceneY - m_selectionRect->y());

    if (m_clickedElement /* && !m_clickedElement->isRest() */) {

        if (m_startedFineDrag) {
            dragFine(e->sceneX, e->sceneY, false);
        } else if (m_clickedShift) {
            if (w > 2 || w < -2 || h > 2 || h < -2) {
                dragFine(e->sceneX, e->sceneY, false);
            }
        } else if (w > 3 || w < -3 || h > 3 || h < -3) {
            drag(e->sceneX, e->sceneY, false);
        }

    } else {

        // Qt rectangle dimensions appear to be 1-based
        if (w > 0) ++w;
        else --w;
        if (h > 0) ++h;
        else --h;

        QPointF p0(m_selectionOrigin);
        QPointF p1(e->sceneX, e->sceneY);
        QRectF r = QRectF(p0, p1).normalized();

        m_selectionRect->setRect(r.x() + 0.5, r.y() + 0.5, r.width(), r.height());
        m_selectionRect->show();

//        m_selectionRect->setSize(w, h);
        setViewCurrentSelection(true);
//        m_nParentView->canvas()->update();
    }

    return FollowMode(FollowHorizontal | FollowVertical);
}

void NotationSelector::handleMouseRelease(const NotationMouseEvent *e)
{
    NOTATION_DEBUG << "NotationSelector::handleMouseRelease" << endl;
    m_updateRect = false;

    // Test how far we've moved from the original click position -- not
    // how big the rectangle is (if we were dragging an event, the
    // rectangle size will still be zero).

    int w = int(e->sceneX - m_selectionRect->x());
    int h = int(e->sceneY - m_selectionRect->y());

    if (w > -3 && w < 3 && h > -3 && h < 3 && !m_startedFineDrag) {

        if (m_clickedElement != 0 && m_selectedStaff) {
            
            // If we didn't drag out a meaningful area, but _did_
            // click on an individual event, then select just that
            // event

            if (m_selectionToMerge &&
                m_selectionToMerge->getSegment() ==
                m_selectedStaff->getSegment()) {

                // if the event was already part of the selection, we want to
                // remove it
                if (m_selectionToMerge->contains(m_clickedElement->event())) {
                    m_selectionToMerge->removeEvent(m_clickedElement->event());
                } else {
                    m_selectionToMerge->addEvent(m_clickedElement->event());
                }
                
                m_scene->setSelection(m_selectionToMerge, true);
                m_selectionToMerge = 0;

            } else {

                m_scene->setSingleSelectedEvent(m_selectedStaff, m_clickedElement, true);
            }
            /*
                    } else if (m_selectedStaff) {
             
                        // If we clicked on no event but on a staff, move the
                        // insertion cursor to the point where we clicked. 
                        // Actually we only really want this to happen if
                        // we aren't double-clicking -- consider using a timer
                        // to establish whether a double-click is going to happen
             
                        m_nParentView->slotSetInsertCursorPosition(e->x(), (int)e->y());
            */
        } else {
            setViewCurrentSelection(false);
        }

    } else {

        if (m_startedFineDrag) {
            dragFine(e->sceneX, e->sceneY, true);
        } else if (m_clickedElement /* && !m_clickedElement->isRest() */) {
            drag(e->sceneX, e->sceneY, true);
        } else {
            setViewCurrentSelection(false);
        }
    }

    m_clickedElement = 0;
    m_selectionRect->hide();
    m_wholeStaffSelectionComplete = false;
    //!!! m_nParentView->canvas()->update();
}

void NotationSelector::drag(int x, int y, bool final)
{
    NOTATION_DEBUG << "NotationSelector::drag " << x << ", " << y << endl;

    if (!m_clickedElement || !m_selectedStaff)
        return ;

    EventSelection *selection = m_scene->getSelection();
    if (!selection || !selection->contains(m_clickedElement->event())) {
        selection = new EventSelection(m_selectedStaff->getSegment());
        selection->addEvent(m_clickedElement->event());
    }
    m_scene->setSelection(selection, false);

#ifdef NOT_JUST_YET
    LinedStaff *targetStaff = m_nParentView->getStaffForCanvasCoords(x, y);
    if (!targetStaff)
        targetStaff = m_selectedStaff;

    // Calculate time and height

    timeT clickedTime = m_clickedElement->event()->getNotationAbsoluteTime();

    Accidental clickedAccidental = Accidentals::NoAccidental;
    (void)m_clickedElement->event()->get<String>(ACCIDENTAL, clickedAccidental);

    long clickedPitch = 0;
    (void)m_clickedElement->event()->get<Int>(PITCH, clickedPitch);

    long clickedHeight = 0;
    (void)m_clickedElement->event()->get<Int>
    (NotationProperties::HEIGHT_ON_STAFF, clickedHeight);

    Event *clefEvt = 0, *keyEvt = 0;
    Clef clef;
    ::Rosegarden::Key key;

    timeT dragTime = clickedTime;
    double layoutX = m_clickedElement->getLayoutX();
    timeT duration = m_clickedElement->getViewDuration();

    NotationElementList::iterator itr =
        targetStaff->getElementUnderCanvasCoords(x, y, clefEvt, keyEvt);

    if (itr != targetStaff->getViewElementList()->end()) {

        NotationElement *elt = dynamic_cast<NotationElement *>(*itr);
        dragTime = elt->getViewAbsoluteTime();
        layoutX = elt->getLayoutX();

        if (elt->isRest() && duration > 0 && elt->getCanvasItem()) {

            double restX = 0, restWidth = 0;
            elt->getCanvasAirspace(restX, restWidth);

            timeT restDuration = elt->getViewDuration();

            if (restWidth > 0 &&
                    restDuration >= duration * 2) {

                int parts = restDuration / duration;
                double encroachment = x - restX;
                NOTATION_DEBUG << "encroachment is " << encroachment << ", restWidth is " << restWidth << endl;
                int part = (int)((encroachment / restWidth) * parts);
                if (part >= parts)
                    part = parts - 1;

                dragTime += part * restDuration / parts;
                layoutX += part * restWidth / parts +
                           (restX - elt->getCanvasX());
            }
        }
    }

    if (clefEvt)
        clef = Clef(*clefEvt);
    if (keyEvt)
        key = ::Rosegarden::Key(*keyEvt);

    int height = targetStaff->getHeightAtCanvasCoords(x, y);
    int pitch = clickedPitch;

    if (height != clickedHeight)
        pitch =
            Pitch
            (height, clef, key, clickedAccidental).getPerformancePitch();

    if (pitch < clickedPitch) {
        if (height < -10) {
            height = -10;
            pitch = Pitch
                    (height, clef, key, clickedAccidental).getPerformancePitch();
        }
    } else if (pitch > clickedPitch) {
        if (height > 18) {
            height = 18;
            pitch = Pitch
                    (height, clef, key, clickedAccidental).getPerformancePitch();
        }
    }

    bool singleNonNotePreview = !m_clickedElement->isNote() &&
                                selection->getSegmentEvents().size() == 1;

    if (!final && !singleNonNotePreview) {

        if ((pitch != m_lastDragPitch || dragTime != m_lastDragTime) &&
            m_clickedElement->isNote()) {

            m_nParentView->showPreviewNote(targetStaff->getId(),
                                           layoutX, pitch, height,
                                           Note::getNearestNote(duration),
                                           m_clickedElement->isGrace());
            m_lastDragPitch = pitch;
            m_lastDragTime = dragTime;
        }

    } else {

        m_nParentView->clearPreviewNote();

        MacroCommand *command = new MacroCommand(MoveCommand::getGlobalName());
        bool haveSomething = false;

        MoveCommand *mc = 0;
        Event *lastInsertedEvent = 0;

        if (pitch != clickedPitch && m_clickedElement->isNote()) {
            command->addCommand(new TransposeCommand(pitch - clickedPitch,
                                *selection));
            haveSomething = true;
        }

        if (targetStaff != m_selectedStaff) {
            command->addCommand(new MoveAcrossSegmentsCommand
                                (m_selectedStaff->getSegment(),
                                 targetStaff->getSegment(),
                                 dragTime - clickedTime + selection->getStartTime(),
                                 true,
                                 *selection));
            haveSomething = true;
        } else {
            if (dragTime != clickedTime) {
                mc = new MoveCommand
                     (m_selectedStaff->getSegment(),  //!!!sort
                      dragTime - clickedTime, true, *selection);
                command->addCommand(mc);
                haveSomething = true;
            }
        }

        if (haveSomething) {

            m_nParentView->addCommandToHistory(command);

            if (mc && singleNonNotePreview) {

                lastInsertedEvent = mc->getLastInsertedEvent();

                if (lastInsertedEvent) {
                    m_nParentView->setSingleSelectedEvent(targetStaff->getId(),
                                                          lastInsertedEvent);

                    ViewElementList::iterator vli =
                        targetStaff->findEvent(lastInsertedEvent);

                    if (vli != targetStaff->getViewElementList()->end()) {
                        m_clickedElement = dynamic_cast<NotationElement *>(*vli);
                    } else {
                        m_clickedElement = 0;
                    }

                    m_selectionRect->setX(x);
                    m_selectionRect->setY(y);
                }
            }
        } else {
            delete command;
        }
    }
#endif
}

void NotationSelector::dragFine(int x, int y, bool final)
{
    NOTATION_DEBUG << "NotationSelector::drag " << x << ", " << y << endl;

    if (!m_clickedElement || !m_selectedStaff)
        return ;

    EventSelection *selection = m_scene->getSelection();
    if (!selection)
        selection = new EventSelection(m_selectedStaff->getSegment());
    if (!selection->contains(m_clickedElement->event()))
        selection->addEvent(m_clickedElement->event());
    m_scene->setSelection(selection, false);

#ifdef NOT_JUST_YET
    // Fine drag modifies the DISPLACED_X and DISPLACED_Y properties on
    // each event.  The modifications have to be relative to the previous
    // values of these properties, not to zero, so for each event we need
    // to store the previous value at the time the drag starts.

    static PropertyName xProperty("temporary-displaced-x");
    static PropertyName yProperty("temporary-displaced-y");

    if (!m_startedFineDrag) {
        // back up original properties

        for (EventSelection::eventcontainer::iterator i =
                    selection->getSegmentEvents().begin();
                i != selection->getSegmentEvents().end(); ++i) {
            long prevX = 0, prevY = 0;
            (*i)->get<Int>(DISPLACED_X, prevX);
            (*i)->get<Int>(DISPLACED_Y, prevY);
            (*i)->setMaybe<Int>(xProperty, prevX);
            (*i)->setMaybe<Int>(yProperty, prevY);
        }

        m_startedFineDrag = true;
    }

    // We want the displacements in 1/1000ths of a staff space

    double dx = x - m_selectionRect->x();
    double dy = y - m_selectionRect->y();

    double noteBodyWidth = m_nParentView->getNotePixmapFactory()->getNoteBodyWidth();
    double lineSpacing = m_nParentView->getNotePixmapFactory()->getLineSpacing();
    dx = (1000.0 * dx) / noteBodyWidth;
    dy = (1000.0 * dy) / lineSpacing;

    if (final) {

        // reset original values (and remove backup values) before
        // applying command

        for (EventSelection::eventcontainer::iterator i =
                    selection->getSegmentEvents().begin();
                i != selection->getSegmentEvents().end(); ++i) {
            long prevX = 0, prevY = 0;
            (*i)->get
            <Int>(xProperty, prevX);
            (*i)->get
            <Int>(yProperty, prevY);
            (*i)->setMaybe<Int>(DISPLACED_X, prevX);
            (*i)->setMaybe<Int>(DISPLACED_Y, prevY);
            (*i)->unset(xProperty);
            (*i)->unset(yProperty);
        }

        IncrementDisplacementsCommand *command = new IncrementDisplacementsCommand
                (*selection, long(dx), long(dy));
        m_nParentView->addCommandToHistory(command);

    } else {

        timeT startTime = 0, endTime = 0;

        for (EventSelection::eventcontainer::iterator i =
                    selection->getSegmentEvents().begin();
                i != selection->getSegmentEvents().end(); ++i) {
            long prevX = 0, prevY = 0;
            (*i)->get
            <Int>(xProperty, prevX);
            (*i)->get
            <Int>(yProperty, prevY);
            (*i)->setMaybe<Int>(DISPLACED_X, prevX + long(dx));
            (*i)->setMaybe<Int>(DISPLACED_Y, prevY + long(dy));
            if (i == selection->getSegmentEvents().begin()) {
                startTime = (*i)->getAbsoluteTime();
            }
            endTime = (*i)->getAbsoluteTime() + (*i)->getDuration();
        }

        if (startTime == endTime)
            ++endTime;
        selection->getSegment().updateRefreshStatuses(startTime, endTime);
        m_nParentView->update();
    }

#endif
}

void NotationSelector::ready()
{
//!!!    m_selectionRect = new Q3CanvasRectangle(m_nParentView->canvas());

//    if (m_selectionRect) {
//        m_selectionRect->hide();
//       m_selectionRect->setPen(GUIPalette::getColour(GUIPalette::SelectionRectangle));

    m_widget->setCanvasCursor(Qt::arrowCursor);
    //!!!??? m_widget->setHeightTracking(false);
}

void NotationSelector::stow()
{
    delete m_selectionRect;
    m_selectionRect = 0;
    //m_nParentView->canvas()->update();
}

void NotationSelector::handleEventRemoved(Event *e)
{
    if (m_clickedElement && m_clickedElement->event() == e) {
        m_clickedElement = 0;
    }
}

void NotationSelector::slotHideSelection()
{
    if (!m_selectionRect) return;
    m_selectionRect->hide();
//    m_selectionRect->setSize(0, 0);
//    m_nParentView->canvas()->update();
}

void NotationSelector::slotInsertSelected()
{
    invokeInParentView("draw");
}

void NotationSelector::slotEraseSelected()
{
    invokeInParentView("erase");
}

void NotationSelector::slotCollapseRestsHard()
{
    invokeInParentView("collapse_rests_aggressively");
}

void NotationSelector::slotRespellFlat()
{
    invokeInParentView("respell_flat");
}

void NotationSelector::slotRespellSharp()
{
    invokeInParentView("respell_sharp");
}

void NotationSelector::slotRespellNatural()
{
    invokeInParentView("respell_natural");
}

void NotationSelector::slotCollapseNotes()
{
    invokeInParentView("collapse_notes");
}

void NotationSelector::slotInterpret()
{
    invokeInParentView("interpret");
}

void NotationSelector::slotStaffAbove()
{
    invokeInParentView("move_events_up_staff");
}

void NotationSelector::slotStaffBelow()
{
    invokeInParentView("move_events_down_staff");
}

void NotationSelector::slotMakeInvisible()
{
    invokeInParentView("make_invisible");
}

void NotationSelector::slotMakeVisible()
{
    invokeInParentView("make_visible");
}

void NotationSelector::setViewCurrentSelection(bool preview)
{
    EventSelection *selection = getEventsInSelectionRect();

    if (m_selectionToMerge) {
        if (selection &&
            m_selectionToMerge->getSegment() == selection->getSegment()) {
            selection->addFromSelection(m_selectionToMerge);
        } else {
            return;
        }
    }

    m_scene->setSelection(selection, preview);
}
/*!!!
NotationStaff *
NotationSelector::getStaffForElement(NotationElement *elt)
{
    for (int i = 0; i < m_nParentView->getStaffCount(); ++i) {
        NotationStaff *staff = m_nParentView->getNotationStaff(i);
        if (staff->getSegment().findSingle(elt->event()) !=
                staff->getSegment().end())
            return staff;
    }
    return 0;
}
*/
EventSelection *
NotationSelector::getEventsInSelectionRect()
{
    // If selection rect is not visible or too small,
    // return 0
    //
    if (!m_selectionRect->isVisible()) return 0;

    if (!m_selectedStaff) return 0;

    Profiler profiler("NotationSelector::getEventsInSelectionRect");

    //    NOTATION_DEBUG << "Selection x,y: " << m_selectionRect->x() << ","
    //                         << m_selectionRect->y() << "; w,h: " << m_selectionRect->width() << "," << m_selectionRect->height() << endl;

    QRectF rect = m_selectionRect->rect();

    if (rect.width()  > -3 &&
        rect.width()  <  3 &&
        rect.height() > -3 &&
        rect.height() <  3) return 0;

    QList<QGraphicsItem *> l = m_selectionRect->collidingItems
        (Qt::IntersectsItemShape);

    Segment& segment = m_selectedStaff->getSegment();

    // If we selected the whole staff, force that to happen explicitly
    // rather than relying on collisions with the rectangle -- because
    // events way off the currently visible area might not even have
    // been drawn yet, and so will not appear in the collision list.
    // (We did still need the collision list to determine which staff
    // to use though.)
    
    if (m_wholeStaffSelectionComplete) {
        EventSelection *selection = new EventSelection(segment,
                                                       segment.getStartTime(),
                                                       segment.getEndMarkerTime());
        return selection;
    }
    
    EventSelection *selection = new EventSelection(segment);
    int nbw = m_selectedStaff->getNotePixmapFactory(false).getNoteBodyWidth();

    for (int i = 0; i < l.size(); ++i) {

        QGraphicsItem *item = l[i];
        NotationElement *element = NotationElement::getNotationElement(item);
        if (!element) continue;
            
        double x = element->getSceneX();
        double y = element->getSceneY();

        bool shifted = false;

        // #957364 (Notation: Hard to select upper note in chords
        // of seconds) -- adjust x-coord for shifted note head
        if (element->event()->get<Bool>(m_selectedStaff->getProperties().
                                       NOTE_HEAD_SHIFTED, shifted) && shifted) {
            x += nbw;
        }

        // check if the element's rect
        // is actually included in the selection rect.
        //
        if (!rect.contains(x, y))  {
            // #988217 (Notation: Special column of pixels
            // prevents sweep selection) -- for notes, test again
            // with centred x-coord
            if (!element->isNote() || !rect.contains(x + nbw/2, y)) {
                continue;
            }
        }
                
        // must be in the same segment as we first started on,
        // we can't select events across multiple segments
        if (selection->getSegment().findSingle(element->event()) !=
            selection->getSegment().end()) {
            selection->addEvent(element->event());
        }
    }

    if (selection->getAddedEvents() > 0) {
        return selection;
    } else {
        delete selection;
        return 0;
    }
}

const QString NotationSelector::ToolName = "notationselector";

}
#include "NotationSelector.moc"


