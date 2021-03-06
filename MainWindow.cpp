/**
* Copyright (C) 2012-2014 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#include <QDir>
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QThread>
#include <QMessageBox>
#include <QFileDialog>
#include <QTreeView>
#include <QtNetwork>


#include "MainWindow.h"
#include "ui_MainWindow.h"

#ifdef Q_OS_WIN
#include "Windows.h"
#include "winioctl.h"
#include "tchar.h"

#include <Setupapi.h>

DEFINE_GUID( GUID_DEVINTERFACE_USB_DISK,
			 0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2,
			 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b );
BOOL EjectVolume(TCHAR cDriveLetter);


HANDLE OpenVolume(TCHAR cDriveLetter);
BOOL LockVolume(HANDLE hVolume);
BOOL DismountVolume(HANDLE hVolume);
BOOL PreventRemovalOfVolume(HANDLE hVolume, BOOL fPrevent);
BOOL AutoEjectVolume(HANDLE hVolume);
BOOL CloseVolume(HANDLE hVolume);

LPTSTR szVolumeFormat = TEXT("\\\\.\\%c:");
LPTSTR szRootFormat = TEXT("%c:\\");
LPTSTR szErrorFormat = TEXT("Error %d: %s\n");


#endif

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	setAcceptDrops(true);

	refreshList();
	ui->listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
	ui->usbDrives->setSelectionMode(QAbstractItemView::MultiSelection);
	ui->listWidget->setFocus();
	ui->listWidget->setAcceptDrops(true);
	_nbThreads = 0;

	_tRefresh.setInterval(1000);
	_tRefresh.start();
	qDebug() << connect(&_tRefresh, SIGNAL(timeout()), this, SLOT(refreshList()));

}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
	if (e->mimeData()->hasUrls()) {
		e->acceptProposedAction();
	}
}
bool MainWindow::eventFilter(QObject *, QEvent *event)
{
	qDebug() << event->type();
	/// The event filter catch the following event:
	switch (event->type()) {

	case QEvent::DragEnter: /// - Accept and process a file drop on the window
		event->accept();
		break;
	case QEvent::Drop:
	{
		const QMimeData* mimeData = static_cast<QDropEvent *>(event)->mimeData();

		// If there is one file (not more) we open it
		if (mimeData->urls().length() == 1) {
			QString filePath = mimeData->urls().first().toLocalFile();
			QString fileType = filePath.split(".").last().toLower();
			ui->listWidget->addItem(fileType);
		}
		break;
	}
	default:
		break;
	}

	return true;
}

void MainWindow::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData = event->mimeData();

	// check for our needed mime type, here a file or a list of files
	if (mimeData->hasUrls())
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		// extract the local paths of the files
		for (int i = 0; i < urlList.size() && i < 32; ++i)
		{
			pathList.append(urlList.at(i).toLocalFile());
		}

		// call a function to open the files
		ui->listWidget->addItems(pathList);
	}
}


MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButton_clicked()
{

	_t.start();

	qlonglong size = 0;
	for(int i = 0; i < ui->listWidget->count(); i++) {
		QFileInfo f(ui->listWidget->item(i)->text());
		if(f.isDir())
			size += Copier::calcSize(ui->listWidget->item(i)->text());
		else
			size += QFile(f.absoluteFilePath()).size();
	}
	qDebug() << "Size :" << size;
	QStringList listOfDrives;
	foreach(QListWidgetItem *dest, ui->usbDrives->selectedItems())
	{
		listOfDrives.append(dest->text());
	}
	QStringList devToCheck;


#ifndef Q_OS_WIN
	QProcess p1;
	p1.start("diskutil",QStringList() << "list");
	p1.waitForFinished();

	QStringList outputCommand;
	foreach(QString line, QString(p1.readAll()).split("\n"))
		outputCommand.append(line);


	foreach (QString line, outputCommand) {
		foreach (QString device, listOfDrives) {
			if(line.contains(device.split("/Volumes/").last()))
				devToCheck.append(line.split(" ").last());
		}
	}
#endif

#ifdef Q_OS_WIN
	devToCheck = listOfDrives;
#endif

	QStringList deviceWithNotEnoughPlace;
	foreach(QString device, devToCheck)
	{
		QProcess p;
#ifndef Q_OS_WIN
        p.start("diskutil", QStringList() << "info" << device);
#else
        p.start("fsutil", QStringList() << "volume" << "diskfree" << device);
#endif
		p.waitForFinished();
		foreach(QString line, QString(p.readAll()).split("\n"))
		{
#ifndef Q_OS_WIN
			if(line.contains("Volume Free Space:"))
			{
				if(line.split("(").at(1).split(" ").first().toLongLong() < size);
				{
					qDebug() << "The device" << device << "has only" << line.split("(").at(1).split(" ").first() << "bytes left";
					qDebug() << (line.split("(").at(1).split(" ").first().toLongLong() < size) << line.split("(").at(1).split(" ").first() << size;
					deviceWithNotEnoughPlace += device;
				}
			}
#else
            qDebug() << "Line --->" << line;
			if(line.contains("disponibles"))
			{
				if(line.split(":").last().toLongLong() < size);
				{
					qDebug() << "The device" << device << "has only" << line.split(":").last().toLongLong() << "bytes left";
					deviceWithNotEnoughPlace += device;
				}
			}
#endif
		}
	}

	if(!deviceWithNotEnoughPlace.isEmpty())
	{
		QMessageBox msg;
		QString message = "Not enough space on the following device(s) : ";
		foreach (QString s, deviceWithNotEnoughPlace) {
			message += "\n" + s;
		}
		message += "\nCopy have been aborted.";
		msg.setText(QString(message));
		msg.setStandardButtons(QMessageBox::Ok);
		msg.setButtonText(QMessageBox::Ok, "Oups...");
		msg.setDefaultButton(QMessageBox::Ok);
		msg.exec();
		return;
	}





	for(int i = 0; i < ui->listWidget->count(); i++) {
		foreach(QListWidgetItem *dest, ui->usbDrives->selectedItems())
		{
			QString src = ui->listWidget->item(i)->text();
			QString dst = "";
			if(ui->radio_folder_yes->isChecked() && ui->new_folder->text() != "") {
				dst = "";
				QString newFolder = "";
				if(ui->new_folder->text().at(0) == '/') {
					dst = dest->text()+ui->new_folder->text().mid(1);
					newFolder = ui->new_folder->text().mid(1);
				}
				else {
					dst = dest->text()+ui->new_folder->text();
					newFolder = ui->new_folder->text();
				}

				if(!QDir(dst).exists()) {
					QDir(dest->text()).mkpath(newFolder);
				}
				else {
					qDebug() << "Existe : " + dst;
				}
				dst += "/" + QFileInfo(ui->listWidget->item(i)->text()).fileName();
			}
			else
				dst = dest->text()+ "/" + QFileInfo(ui->listWidget->item(i)->text()).fileName();


			qDebug() << "Copy from" << src << "to" << dst;
			QThread *thread = new QThread;
			_nbThreads++;

			Copier * c = new Copier(src, dst, &_p);
			c->moveToThread(thread);
			connect(c, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
			connect(thread, SIGNAL(started()), c, SLOT(process()));
			connect(c, SIGNAL(finished()), thread, SLOT(quit()));
			connect(c, SIGNAL(finished()), c, SLOT(deleteLater()));
			connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
			connect(thread, SIGNAL(finished()), this, SLOT(on_finnish()));
			_p.addCopy(src, dst);
			_p.show();
			thread->start(QThread::HighestPriority);

		}
	}


}

void MainWindow::on_finnish()
{
	_nbThreads--;
	qDebug() << _t.elapsed();
	if(_nbThreads == 0)
	{
		if(ui->address_mail->text() != "") {
			const QUrl url = QUrl("http://pierrickakosz.com/duplicator/send_mail.php?dest=" + ui->address_mail->text());
			const QNetworkRequest requete(url);
			QNetworkAccessManager *m = new QNetworkAccessManager;
			m->get(requete);
		}
		qDebug() << "finish ! " << _t.elapsed();
		_p.hide();
	}
}


void MainWindow::on_usbDrives_itemActivated(QListWidgetItem *item)
{
	qDebug() << item->text();
}

void MainWindow::on_pb_Clear_clicked()
{
	ui->listWidget->clear();
}

void MainWindow::on_pb_Select_clicked()
{
	if(ui->pb_Select->text() == "Select All")
	{
		ui->usbDrives->selectAll();
		ui->pb_Select->setText("Clear selection");
	}
	else
	{
		ui->usbDrives->clearSelection();
		ui->pb_Select->setText("Select All");
	}
}

void MainWindow::errorString(QString str)
{
	QMessageBox msgBox;
	msgBox.setText("Error");
	msgBox.setInformativeText(str);

	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.exec();
}

void MainWindow::on_pushButton_2_clicked()
{
	foreach(QListWidgetItem *dest, ui->usbDrives->selectedItems())
	{
		QString destination = dest->text();
		qDebug() << "Ask permission for erasing" << destination;

		QMessageBox msgBox;
		msgBox.setText("Information");
		msgBox.setInformativeText("Are you sure you want to erase all content on " + destination);


		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		msgBox.exec();
		if(msgBox.result() == QDialog::Accepted or msgBox.result() == QMessageBox::Ok)
		{
			qDebug() << "Start erasing" << destination;
			QMessageBox msgBox;

			Copier::rmDir(destination);

			msgBox.setText("The volume \"" + destination.split("/").last() + "\" have been erased.");

			msgBox.exec();

		}
	}
}

void MainWindow::on_pb_Eject_clicked()
{
#ifdef Q_OS_WIN
	foreach(QListWidgetItem *dest, ui->usbDrives->selectedItems())
	{
		qDebug() << "eject :" << dest->text();
		std::string str = QString(dest->text().at(0)).toStdString();
		TCHAR * param = new TCHAR[str.size()+1];
		param[str.size()]=0;
		std::copy(str.begin(),str.end(),param);
		EjectVolume(*param);
	}
#else
	foreach(QListWidgetItem *dest, ui->usbDrives->selectedItems())
	{
		qDebug() << "Eject" << dest->text();
		QString command = "osascript -e 'tell application \"Finder\" \n eject the disk \"" + dest->text().split("/").last() + "\" \n end tell'";
		qDebug() << command;
		system(command.toStdString().c_str());
	}
#endif

	refreshList();
}

void MainWindow::refreshList()
{
	QVector<QString> selection;
	foreach (QListWidgetItem * i, ui->usbDrives->selectedItems()) {
		selection.append(i->text());
	}
	ui->usbDrives->clear();
#ifdef Q_OS_WIN

	foreach( QFileInfo drive, QDir::drives() )
	{
		ui->usbDrives->addItem(drive.absoluteFilePath());
	}
#else

	QDir dir("/Volumes/");

	QFileInfoList list = dir.entryInfoList();

	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		if(fileInfo.absolutePath() != "/" and fileInfo.absolutePath() != "/Volume")
			ui->usbDrives->addItem(fileInfo.absoluteFilePath());
	}
#endif

	//qDebug() << selection;
	for(int i = 0; i < ui->usbDrives->count(); i++) {

		if(selection.contains(ui->usbDrives->item(i)->text()))
			ui->usbDrives->item(i)->setSelected(true);
	}
}

#ifdef Q_OS_WIN
void ReportError(LPTSTR szMsg)
{
	//_tprintf(szErrorFormat, GetLastError(), szMsg);
}

HANDLE OpenVolume(TCHAR cDriveLetter)
{
	HANDLE hVolume;
	UINT uDriveType;
	TCHAR szVolumeName[8];
	TCHAR szRootName[5];
	DWORD dwAccessFlags;

	wsprintf(szRootName, szRootFormat, cDriveLetter);

	uDriveType = GetDriveType(szRootName);
	switch(uDriveType) {
	case DRIVE_REMOVABLE:
		break;
	case DRIVE_NO_ROOT_DIR:
		dwAccessFlags = GENERIC_READ | GENERIC_WRITE;
		break;
	case DRIVE_CDROM:
		dwAccessFlags = GENERIC_READ;
		break;
	default:
		qDebug() << ("Cannot eject.  Drive type is incorrect.\n");
		return INVALID_HANDLE_VALUE;
	}

	wsprintf(szVolumeName, szVolumeFormat, cDriveLetter);

	hVolume = CreateFile(   szVolumeName,
							dwAccessFlags,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							0,
							NULL );
	if (hVolume == INVALID_HANDLE_VALUE)
		ReportError(TEXT("CreateFile"));

	return hVolume;
}

BOOL CloseVolume(HANDLE hVolume)
{
	return CloseHandle(hVolume);
}

#define LOCK_TIMEOUT        10000       // 10 Seconds
#define LOCK_RETRIES        20

BOOL LockVolume(HANDLE hVolume)
{
	DWORD dwBytesReturned;
	DWORD dwSleepAmount;
	int nTryCount;

	dwSleepAmount = LOCK_TIMEOUT / LOCK_RETRIES;

	// Do this in a loop until a timeout period has expired
	for (nTryCount = 0; nTryCount < LOCK_RETRIES; nTryCount++) {
		if (DeviceIoControl(hVolume,
							FSCTL_LOCK_VOLUME,
							NULL, 0,
							NULL, 0,
							&dwBytesReturned,
							NULL))
			return TRUE;

		Sleep(dwSleepAmount);
	}

	return FALSE;
}

BOOL DismountVolume(HANDLE hVolume)
{
	DWORD dwBytesReturned;

	return DeviceIoControl( hVolume,
							FSCTL_DISMOUNT_VOLUME,
							NULL, 0,
							NULL, 0,
							&dwBytesReturned,
							NULL);
}

BOOL PreventRemovalOfVolume(HANDLE hVolume, BOOL fPreventRemoval)
{
	DWORD dwBytesReturned;
	PREVENT_MEDIA_REMOVAL PMRBuffer;

	PMRBuffer.PreventMediaRemoval = fPreventRemoval;

	return DeviceIoControl( hVolume,
							IOCTL_STORAGE_MEDIA_REMOVAL,
							&PMRBuffer, sizeof(PREVENT_MEDIA_REMOVAL),
							NULL, 0,
							&dwBytesReturned,
							NULL);
}

BOOL AutoEjectVolume(HANDLE hVolume)
{
	DWORD dwBytesReturned;

	return DeviceIoControl( hVolume,
							IOCTL_STORAGE_EJECT_MEDIA,
							NULL, 0,
							NULL, 0,
							&dwBytesReturned,
							NULL);
}

BOOL EjectVolume(TCHAR cDriveLetter)
{
	HANDLE hVolume;

	BOOL fRemoveSafely = FALSE;
	BOOL fAutoEject = FALSE;

	// Open the volume.
	hVolume = OpenVolume(cDriveLetter);
	if (hVolume == INVALID_HANDLE_VALUE)
		return FALSE;

	// Lock and dismount the volume.
	if (LockVolume(hVolume) && DismountVolume(hVolume)) {
		fRemoveSafely = TRUE;

		// Set prevent removal to false and eject the volume.
		if (PreventRemovalOfVolume(hVolume, FALSE) &&
				AutoEjectVolume(hVolume))
			fAutoEject = TRUE;
	}

	// Close the volume so other processes can use the drive.
	if (!CloseVolume(hVolume))
		return FALSE;

	if (fAutoEject)
		qDebug() << "Media in Drive %c has been ejected safely.\n" <<
					cDriveLetter;
	else {
		if (fRemoveSafely)
			qDebug() << "Media in Drive %c can be safely removed.\n" <<
						cDriveLetter;
	}

	return TRUE;
}

void Usage()
{
	//qDebug() << "Usage: Eject <drive letter>\n\n");
	return ;
}
#endif


void MainWindow::on_pb_browse_clicked()
{
	QFileDialog dialog(this);
	dialog.setDirectory(QDir::homePath());
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setFileMode(QFileDialog::ExistingFiles);

	QStringList fileNames;
	if (dialog.exec()) {
		fileNames = dialog.selectedFiles();
		ui->listWidget->addItems(fileNames);
	}
}

void MainWindow::on_pb_browse_folders_clicked()
{
	QFileDialog* dialog = new QFileDialog(this);
	dialog->setFileMode(QFileDialog::Directory);
	dialog->setOption(QFileDialog::DontUseNativeDialog, true);

	// Try to select multiple files and directories at the same time in QFileDialog
	QListView *l = dialog->findChild<QListView*>("listView");
	if (l) {
		l->setSelectionMode(QAbstractItemView::ExtendedSelection);
	}
	QTreeView *t = dialog->findChild<QTreeView*>();
	if (t) {
		t->setSelectionMode(QAbstractItemView::ExtendedSelection);
	}
	if (dialog->exec()) {
		QStringList _fnames = dialog->selectedFiles();
		ui->listWidget->addItems(_fnames);
	}
}

void MainWindow::on_radio_folder_yes_clicked()
{
	ui->new_folder->setEnabled(true);
}

void MainWindow::on_radio_folder_no_clicked()
{
	ui->new_folder->setEnabled(false);
}


void MainWindow::on_actionAbout_triggered()
{
	QMessageBox msg;
	msg.setText(QString("The application have been coded by :\n\t Thomas Ayoub, Pierrick Akosz.\n\n")+
				QString("The fantastic design is the masterpiece of :\n\t Océane Deflandre and Yanis Boulaouinat.\n\n")+
				QString("Under the direction of the incredible project manager :\n\t Damien Neri."));
	msg.setStandardButtons(QMessageBox::Ok);
	msg.setButtonText(QMessageBox::Ok, "Youpi!");
	msg.setDefaultButton(QMessageBox::Ok);
	msg.exec();
}
