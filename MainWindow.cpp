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


#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	setAcceptDrops(true);

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
		ui->usbDrives->addItem(fileInfo.absoluteFilePath());
	}
#endif
    ui->listWidget->setSelectionMode(QAbstractItemView::MultiSelection);
	ui->usbDrives->setSelectionMode(QAbstractItemView::MultiSelection);
	ui->listWidget->setFocus();
	ui->listWidget->setAcceptDrops(true);
	_nbThreads = 0;



}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
	if (e->mimeData()->hasUrls()) {
		e->acceptProposedAction();
	}
}
bool MainWindow::eventFilter(QObject * sender, QEvent *event)
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

	for(int i = 0; i < ui->listWidget->count(); i++) {
		foreach(QListWidgetItem *dest, ui->usbDrives->selectedItems())
		{
			QString src = ui->listWidget->item(i)->text();
			QString dst = dest->text()+ "/" + QFileInfo(ui->listWidget->item(i)->text()).fileName();
			qDebug() << "Copy from" << src << "to" << dst;
			QThread *thread = new QThread;
			_nbThreads++;

			Copier * c = new Copier(src, dst);
			c->moveToThread(thread);
			connect(c, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
			connect(thread, SIGNAL(started()), c, SLOT(process()));
			connect(c, SIGNAL(finished()), thread, SLOT(quit()));
			connect(c, SIGNAL(finished()), c, SLOT(deleteLater()));
			connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
			connect(thread, SIGNAL(finished()), this, SLOT(on_finnish()));
			_p.addCopy(src, dst);
			_p.show();
			thread->start(QThread::HighPriority);
		}
	}


}

void MainWindow::on_finnish()
{
	_nbThreads--;
	qDebug() << _t.elapsed();
	if(_nbThreads == 0)
	{
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
		QMessageBox msgBox;
		msgBox.setText("Information");
		msgBox.setInformativeText("Are you sure you want to erase all content on " + dest->text());

		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		if(msgBox.exec())
		{
			qDebug() << "Start erasing";
			QMessageBox msgBox;

			if(Copier::rmDir(dest->text()))
			{
				msgBox.setText("The volume \"" + dest->text().split("/").last() + "\" have been erased.");
				msgBox.exec();
			}

		}

	}
}
