#ifndef STABLEHEADERS_H_
#define STABLEHEADERS_H_

// Standard C++ library
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <exception>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <strstream>
#include <utility>
#include <vector>

// QT3 headers

// Common headers, or used by sources generated by moc
#include <QMap> 
#include <qglobal.h>
#include <private/qucomextra_p.h> 
#include <QMetaObject>
#include <qobjectdefs.h>
#include <qsignalslotimp.h>
#include <QStyle>

// Headers used by Rosegarden or KDE3
#include <qshortcut.h>
#include <QApplication>
#include <QBitmap>
#include <QBrush>
#include <QBuffer>
#include <QGroupBox>
#include <qbutton.h>
#include <Q3Canvas>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QByteArray>
#include <QCursor>
#include <QDataStream>
#include <QDateTime>
#include <QDialog>
#include <QMap>
#include <QDir>
#include <qdom.h>
#include <qdragobject.h>
#include <qdrawutil.h>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontInfo>
#include <QFontMetrics>
#include <QFrame>
#include <qgarray.h>
#include <qgrid.h>
#include <QGroupBox>
#include <qguardedptr.h>
#include <qheader.h>
#include <QIcon>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <qlistbox.h>
#include <QListWidget>
#include <qmemarray.h>
#include <QMutex>
#include <QObject>
#include <QObjectList>
#include <qpaintdevicemetrics.h>
#include <QPainter>
#include <QPalette>
#include <QPen>
#include <QPixmap>
#include <qpointarray.h>
#include <QPoint>
#include <qpopupmenu.h>
#include <QPrinter>
#include <QProgressDialog>
#include <qptrdict.h>
#include <qptrlist.h>
#include <QPushButton>
#include <QRadioButton>
#include <QRect>
#include <QRegExp>
#include <QRegion>
#include <QScrollBar>
#include <qscrollview.h>
#include <QSessionManager>
#include <QSignalMapper>
#include <QSize>
#include <QSizePolicy>
#include <QSlider>
#include <QSocketNotifier>
#include <QSpinBox>
#include <QSplitter>
#include <QString>
#include <QStringList>
#include <QStringList>
#include <qtable.h>
#include <QTabWidget>
#include <QTextCodec>
#include <QTextEdit>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QValidator>
#include <QLinkedList>
#include <QVector>
#include <QVariant>
#include <qvgroupbox.h>
#include <QWhatsThis>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QMatrix>
#include <qxml.h>

// KDE3 headers
#include <dcopclient.h>
#include <dcopobject.h>
#include <dcopref.h>
#include <kaboutdata.h>
#include <kshortcut.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kapp.h>
#include <QApplication>
#include <karrowbutton.h>
#include <kcmdlineargs.h>
#include <kcolordialog.h>
#include <QComboBox>
#include "document/Command.h"
#include <kcompletion.h>
#include <QSettings>
#include <kcursor.h>
#include <kdcopactionproxy.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kdialogbase.h>
#include <kdialog.h>
#include <kdiskfreesp.h>
#include <QDockWidget>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kfile.h>
#include <kfilterdev.h>
#include <kfontrequester.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>
#include <kkeydialog.h>
#include <kled.h>
#include <klineeditdlg.h>
#include <klineedit.h>
#include <QListWidget>
#include <klocale.h>
#include <kmainwindow.h>
#include <QMessageBox>
#include <kmimetype.h>
#include <kpixmapeffect.h>
#include <kpopupmenu.h>
#include <kprinter.h>
#include <QProcess>
#include <kpushbutton.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktempfile.h>
#include <ktip.h>
#include <ktoolbar.h>
#include <kuniqueapplication.h>
#include <kurl.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

#define private protected // fugly
#include <kvalue().h>
#undef private

#endif /*STABLEHEADERS_H_*/
