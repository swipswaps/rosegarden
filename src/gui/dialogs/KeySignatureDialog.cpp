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


#include "KeySignatureDialog.h"

#include <klocale.h>
#include "misc/Strings.h"
#include "base/NotationTypes.h"
#include "gui/editors/notation/NotePixmapFactory.h"
#include "gui/widgets/BigArrowButton.h"
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QObject>
#include <QPixmap>
#include <QRadioButton>
#include <QString>
#include <QToolTip>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <algorithm>

namespace Rosegarden
{

KeySignatureDialog::KeySignatureDialog(QWidget *parent,
                                       NotePixmapFactory *npf,
                                       Clef clef,
                                       Rosegarden::Key defaultKey,
                                       bool showApplyToAll,
                                       bool showConversionOptions,
                                       QString explanatoryText) :
        QDialog(parent),
        m_notePixmapFactory(npf),
        m_key(defaultKey),
        m_clef(clef),
        m_valid(true),
        m_ignoreComboChanges(false),
        m_explanatoryLabel(0),
        m_applyToAllButton(0),
        m_noPercussionCheckBox(0)
{
    //setHelp("nv-signatures-key");

    setModal(true);
    setWindowTitle(QObject::tr("Key Change"));

    QGridLayout *metagrid = new QGridLayout;
    setLayout(metagrid);
    QWidget *vbox = new QWidget(this);
    QVBoxLayout *vboxLayout = new QVBoxLayout;
    metagrid->addWidget(vbox, 0, 0);


    QWidget *keyBox = 0;
    QWidget *nameBox = 0;

    QGroupBox *keyFrame = new QGroupBox( QObject::tr("Key signature"), vbox );
    QVBoxLayout *keyFrameLayout = new QVBoxLayout;
    vboxLayout->addWidget(keyFrame);

    QGroupBox *transposeFrame = new QGroupBox( QObject::tr("Key transposition"), vbox);
    QVBoxLayout *transposeFrameLayout = new QVBoxLayout;
    vboxLayout->addWidget(transposeFrame);

    QGroupBox *buttonFrame = new QGroupBox(QObject::tr("Scope"), vbox);
    QVBoxLayout *buttonFrameLayout = new QVBoxLayout;
    vboxLayout->addWidget(buttonFrame);

    QGroupBox *conversionFrame = new QGroupBox( QObject::tr("Existing notes following key change"), vbox );
    QVBoxLayout *conversionFrameLayout = new QVBoxLayout;
    vboxLayout->addWidget(conversionFrame);
    vbox->setLayout(vboxLayout);

    keyBox = new QWidget(keyFrame);
    QHBoxLayout *keyBoxLayout = new QHBoxLayout;
    keyFrameLayout->addWidget(keyBox);
    nameBox = new QWidget(keyFrame);
    QHBoxLayout *nameBoxLayout = new QHBoxLayout;
    keyFrameLayout->addWidget(nameBox);

    QLabel *explanatoryLabel = 0;
    if (!explanatoryText.isEmpty()) {
        explanatoryLabel = new QLabel(explanatoryText, keyFrame);
        keyFrameLayout->addWidget(explanatoryLabel);
    }

    keyFrame->setLayout(keyFrameLayout);

    BigArrowButton *keyDown = new BigArrowButton(keyBox, Qt::LeftArrow);
    keyBoxLayout->addWidget(keyDown);
    keyDown->setToolTip(QObject::tr("Flatten"));

    m_keyLabel = new QLabel(QObject::tr("Key"), keyBox);
    keyBoxLayout->addWidget(m_keyLabel);
    m_keyLabel->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter);

    BigArrowButton *keyUp = new BigArrowButton(keyBox, Qt::RightArrow);
    keyBoxLayout->addWidget(keyUp);
    keyBox->setLayout(keyBoxLayout);
    keyUp->setToolTip(QObject::tr("Sharpen"));

    m_keyCombo = new QComboBox(nameBox);
    nameBoxLayout->addWidget(m_keyCombo);
    m_majorMinorCombo = new QComboBox(nameBox);
    nameBoxLayout->addWidget(m_majorMinorCombo);
    nameBox->setLayout(nameBoxLayout);
    m_majorMinorCombo->addItem(QObject::tr("Major"));
    m_majorMinorCombo->addItem(QObject::tr("Minor"));
    if (m_key.isMinor()) {
        m_majorMinorCombo->setCurrentIndex(m_majorMinorCombo->count() - 1);
    }

    regenerateKeyCombo();
    redrawKeyPixmap();
    m_explanatoryLabel = explanatoryLabel;

    m_keyLabel->setMinimumWidth(m_keyLabel->pixmap()->width());
    m_keyLabel->setMinimumHeight(m_keyLabel->pixmap()->height());

    m_yesTransposeButton =
        new QRadioButton(QObject::tr("Transpose key according to segment transposition"),
                         transposeFrame);
    transposeFrameLayout->addWidget(m_yesTransposeButton);
    QRadioButton *noTransposeButton =
        new QRadioButton(QObject::tr("Use specified key.  Do not transpose"), transposeFrame);
    transposeFrameLayout->addWidget(noTransposeButton);
    m_yesTransposeButton->setChecked(true);

    // just to shut up the compiler warning about unused variable:
    noTransposeButton->setChecked(false);

    transposeFrame->setLayout(transposeFrameLayout);

    if (showApplyToAll) {
        QRadioButton *applyToOneButton =
            new QRadioButton(QObject::tr("Apply to current segment only"),
                             buttonFrame);
        buttonFrameLayout->addWidget(applyToOneButton);
        m_applyToAllButton =
            new QRadioButton(QObject::tr("Apply to all segments at this time"),
                             buttonFrame);
        buttonFrameLayout->addWidget(m_applyToAllButton);
        applyToOneButton->setChecked(true);
        m_noPercussionCheckBox =
            new QCheckBox(QObject::tr("Exclude percussion segments"), buttonFrame);
        buttonFrameLayout->addWidget(m_noPercussionCheckBox);
        m_noPercussionCheckBox->setChecked(true);
    } else {
        m_applyToAllButton = 0;
        buttonFrame->hide();
    }

    buttonFrame->setLayout(buttonFrameLayout);

    if (showConversionOptions) {
        m_noConversionButton =
            new QRadioButton
            (QObject::tr("Maintain current pitches"), conversionFrame);
        conversionFrameLayout->addWidget(m_noConversionButton);
        m_convertButton =
            new QRadioButton
            (QObject::tr("Maintain current accidentals"), conversionFrame);
        conversionFrameLayout->addWidget(m_convertButton);
        m_transposeButton =
            new QRadioButton
            (QObject::tr("Transpose into this key"), conversionFrame);
        conversionFrameLayout->addWidget(m_transposeButton);
        m_noConversionButton->setChecked(true);
    } else {
        m_noConversionButton = 0;
        m_convertButton = 0;
        m_transposeButton = 0;
        conversionFrame->hide();
    }

    conversionFrame->setLayout(conversionFrameLayout);

    QObject::connect(keyUp, SIGNAL(clicked()), this, SLOT(slotKeyUp()));
    QObject::connect(keyDown, SIGNAL(clicked()), this, SLOT(slotKeyDown()));
    QObject::connect(m_keyCombo, SIGNAL(activated(const QString &)),
                     this, SLOT(slotKeyNameChanged(const QString &)));
    QObject::connect(m_keyCombo, SIGNAL(textChanged(const QString &)),
                     this, SLOT(slotKeyNameChanged(const QString &)));
    QObject::connect(m_majorMinorCombo, SIGNAL(activated(const QString &)),
                     this, SLOT(slotMajorMinorChanged(const QString &)));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
    metagrid->addWidget(buttonBox, 1, 0);
    metagrid->setRowStretch(0, 10);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

KeySignatureDialog::ConversionType

KeySignatureDialog::getConversionType() const
{
    if (m_noConversionButton && m_noConversionButton->isChecked()) {
        return NoConversion;
    } else if (m_convertButton && m_convertButton->isChecked()) {
        return Convert;
    } else if (m_transposeButton && m_transposeButton->isChecked()) {
        return Transpose;
    }
    return NoConversion;
}

bool
KeySignatureDialog::shouldApplyToAll() const
{
    return m_applyToAllButton && m_applyToAllButton->isChecked();
}

bool
KeySignatureDialog::shouldBeTransposed() const
{
    return m_yesTransposeButton && m_yesTransposeButton->isChecked();
}

bool
KeySignatureDialog::shouldIgnorePercussion() const
{
    return m_noPercussionCheckBox && m_noPercussionCheckBox->isChecked();
}

void
KeySignatureDialog::slotKeyUp()
{
    bool sharp = m_key.isSharp();
    int ac = m_key.getAccidentalCount();
    if (ac == 0)
        sharp = true;
    if (sharp) {
        if (++ac > 7)
            ac = 7;
    } else {
        if (--ac < 1) {
            ac = 0;
            sharp = true;
        }
    }

    try {
        m_key = Rosegarden::Key(ac, sharp, m_key.isMinor());
        setValid(true);
    } catch (Rosegarden::Key::BadKeySpec s) {
        std::cerr << s.getMessage() << std::endl;
        setValid(false);
    }

    regenerateKeyCombo();
    redrawKeyPixmap();
}

void
KeySignatureDialog::slotKeyDown()
{
    bool sharp = m_key.isSharp();
    int ac = m_key.getAccidentalCount();
    if (ac == 0)
        sharp = false;
    if (sharp) {
        if (--ac < 0) {
            ac = 1;
            sharp = false;
        }
    } else {
        if (++ac > 7)
            ac = 7;
    }

    try {
        m_key = Rosegarden::Key(ac, sharp, m_key.isMinor());
        setValid(true);
    } catch (Rosegarden::Key::BadKeySpec s) {
        std::cerr << s.getMessage() << std::endl;
        setValid(false);
    }

    regenerateKeyCombo();
    redrawKeyPixmap();
}

struct KeyNameComparator
{
    bool operator()(const Rosegarden::Key &k1, const Rosegarden::Key &k2) {
        return (k1.getName() < k2.getName());
    }
};


void
KeySignatureDialog::regenerateKeyCombo()
{
    if (m_explanatoryLabel)
        m_explanatoryLabel->hide();

    m_ignoreComboChanges = true;
    QString currentText = m_keyCombo->currentText();
    Rosegarden::Key::KeyList keys(Rosegarden::Key::getKeys(m_key.isMinor()));
    m_keyCombo->clear();

    std::sort(keys.begin(), keys.end(), KeyNameComparator());
    bool textSet = false;

    for (Rosegarden::Key::KeyList::iterator i = keys.begin();
            i != keys.end(); ++i) {

        QString name(strtoqstr(i->getName()));
        int space = name.indexOf(' ');
        if (space > 0)
            name = name.left(space);

        m_keyCombo->addItem(name);

        if (m_valid && (*i == m_key)) {
            m_keyCombo->setCurrentIndex(m_keyCombo->count() - 1);
            textSet = true;
        }
    }

    if (!textSet) {
        m_keyCombo->setEditText(currentText);
    }
    m_ignoreComboChanges = false;
}

bool
KeySignatureDialog::isValid() const
{
    return m_valid;
}

Rosegarden::Key
KeySignatureDialog::getKey() const
{
    return m_key;
}

void
KeySignatureDialog::redrawKeyPixmap()
{
    if (m_valid) {
        QPixmap pmap =
            NotePixmapFactory::toQPixmap(m_notePixmapFactory->makeKeyDisplayPixmap(m_key, m_clef));
        m_keyLabel->setPixmap(pmap);
    } else {
        m_keyLabel->setText(QObject::tr("No such key"));
    }
}

void
KeySignatureDialog::slotKeyNameChanged(const QString &s)
{
    if (m_ignoreComboChanges)
        return ;

    if (m_explanatoryLabel)
        m_explanatoryLabel->hide();

    std::string name(getKeyName(s, m_key.isMinor()));

    try {
        m_key = Rosegarden::Key(name);
        setValid(true);

        int space = name.find(' ');
        if (space > 0)
            name = name.substr(0, space);
        m_keyCombo->setEditText(strtoqstr(name));

    } catch (Rosegarden::Key::BadKeyName s) {
        std::cerr << s.getMessage() << std::endl;
        setValid(false);
    }

    redrawKeyPixmap();
}

void
KeySignatureDialog::slotMajorMinorChanged(const QString &s)
{
    if (m_ignoreComboChanges)
        return ;

    std::string name(getKeyName(m_keyCombo->currentText(), s == QObject::tr("Minor")));

    try {
        m_key = Rosegarden::Key(name);
        setValid(true);
    } catch (Rosegarden::Key::BadKeyName s) {
        std::cerr << s.getMessage() << std::endl;
        setValid(false);
    }

    regenerateKeyCombo();
    redrawKeyPixmap();
}

void
KeySignatureDialog::setValid(bool valid)
{
    m_valid = valid;
	//enableButton( Ok, m_valid);	//&&& which button to enable here ????
	//m_valid.setEnabled(true);
}

std::string
KeySignatureDialog::getKeyName(const QString &s, bool minor)
{
    QString u((s.length() >= 1) ? (s.left(1).toUpper() + s.right(s.length() - 1))
              : s);

    std::string name(qstrtostr(u));
    name = name + " " + (minor ? "minor" : "major");
    return name;
}

}
#include "KeySignatureDialog.moc"
