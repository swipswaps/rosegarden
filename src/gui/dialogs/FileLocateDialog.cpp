/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2008 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#include "FileLocateDialog.h"

#include <klocale.h>
#include "misc/Debug.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QString>
#include <QWidget>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>


namespace Rosegarden
{

FileLocateDialog::FileLocateDialog(QWidget *parent,
                                   const QString &file,
                                   const QString & /*path*/):
        QDialog(parent),
        m_file(file)
{
    setModal(true);
    setWindowTitle(i18n("Locate audio file"));
    QGridLayout *metagrid = new QGridLayout;
    setLayout(metagrid);
    QWidget *w = new QWidget(this);
    QHBoxLayout *wLayout = new QHBoxLayout;
    metagrid->addWidget(w, 0, 0);

    QString label =
        i18n("Can't find file \"%1\".\n"
             "Would you like to try and locate this file or skip it?", m_file);

    QLabel *labelW = new QLabel(label, w);
    wLayout->addWidget(labelW);
    labelW->setAlignment(Qt::AlignCenter);
    labelW->setMinimumHeight(60);
    w->setLayout(wLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;

    QPushButton *user1 = new QPushButton(i18n("&Skip"));
    buttonBox->addButton(user1, QDialogButtonBox::ActionRole);
    connect(user1, SIGNAL(clicked(bool)), this, SLOT(slotUser1()));

    QPushButton *user2 = new QPushButton(i18n("Skip &All"));
    buttonBox->addButton(user2, QDialogButtonBox::ActionRole);
    connect(user2, SIGNAL(clicked(bool)), this, SLOT(slotUser2()));

    QPushButton *user3 = new QPushButton(i18n("&Locate"));
    buttonBox->addButton(user3, QDialogButtonBox::ActionRole);
    connect(user3, SIGNAL(clicked(bool)), this, SLOT(slotUser3()));

    metagrid->addWidget(buttonBox, 1, 0);
    metagrid->setRowStretch(0, 10);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void
FileLocateDialog::slotUser3()
{
    if (!m_file.isEmpty()) {
		m_file = QFileDialog::getOpenFileName( this, i18n("Select an Audio File"), QDir::currentPath(), i18n("%1|Requested file (%2)\n*.wav|WAV files (*.wav)", QFileInfo(m_file).fileName(), QFileInfo(m_file).fileName()) );
				
				/*
				(":WAVS",
                  i18n("%1|Requested file (%2)\n*.wav|WAV files (*.wav)",
                   QFileInfo(m_file).fileName(),
                   QFileInfo(m_file).fileName()),
                  this, i18n("Select an Audio File"));
				*/

        RG_DEBUG << "FileLocateDialog::slotUser3() : m_file = " << m_file << endl;

        if (m_file.isEmpty()) {
            RG_DEBUG << "FileLocateDialog::slotUser3() : reject\n";
            reject();
        } else {
            QFileInfo fileInfo(m_file);
            m_path = fileInfo.path();
            accept();
        }

    } else
        reject();
}

void
FileLocateDialog::slotUser1()
{
    reject();
}

void
FileLocateDialog::slotUser2()
{
    done( -1);
}

}
#include "FileLocateDialog.moc"
