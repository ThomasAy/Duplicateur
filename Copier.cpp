/**
* Copyright (C) 2012-2014 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#include "Copier.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QProgressBar>
#include <QMessageBox>
#include <QLayout>
#include <QThread>

Copier::Copier(QObject *parent) :
	QObject(parent)
{
}

Copier::Copier(QString src, QString dest, Progression *window)
{
	_src = src;
	_dest = dest;
	_window = window;
	_nbFiles = 1;
}

void Copier::process(){
	if(QFile(_dest).exists() && _src.at(_src.count() - 1) != '/')
	{
		emit error("The file" + _dest + " already exist!");
	}
	else// if(_src.at(_src.count() - 1) == '/' or true)
	{
		if(_src.at(_src.count() - 1) == '/')
		{
			_nbFiles = countFile(_src);
			qDebug() << "There is" << _nbFiles;
			_window->updateProgressBarMax(_src, _nbFiles);
		}
		cpDir(_src, _dest + "/" + _src.split("/").at(_src.split("/").count() - 2));
	}
	qDebug() << _dest << "copy finished";
	emit finished();
}

bool Copier::cpDir(const QString &srcPath, const QString &dstPath)
{
	qDebug() << "copy from" << srcPath << "to" << dstPath << "using cpdir";
	rmDir(dstPath);
	QDir parentDstDir(QFileInfo(dstPath).path());
	if (!parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
		return false;

	QDir srcDir(srcPath);
	foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
		QString srcItemPath = srcPath + "/" + info.fileName();
		QString dstItemPath = dstPath + "/" + info.fileName();
		if (info.isDir()) {
			if (!cpDir(srcItemPath, dstItemPath)) {
				return false;
			}
		} else if (info.isFile()) {
			qDebug() << "Copy file" << info.fileName();
			_window->incrementProgressBar(_src);
			QThread::msleep(10);
			if (!QFile::copy(srcItemPath, dstItemPath)) {
				return false;
			}
		} else {
			qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
		}
	}
	return true;
}

bool Copier::rmDir(const QString &dirPath)
{
	QDir dir(dirPath);
	if (!dir.exists())
		return true;
	foreach(const QFileInfo &info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
		if (info.isDir()) {
			if (!rmDir(info.filePath()))
				return false;
		} else {
			if (!dir.remove(info.fileName()))
				return false;
			else
				qDebug() << "Removed" << info.fileName();
		}
	}
	QDir parentDir(QFileInfo(dirPath).path());
	return parentDir.rmdir(QFileInfo(dirPath).fileName());
}

int Copier::calcSize(const QString &dirPath)
{
	int sizex = 0;
	QFileInfo str_info(dirPath);
	if (str_info.isDir())
	{
		QDir dir(dirPath);
		QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs |  QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
		for (int i = 0; i < list.size(); ++i)
		{
			QFileInfo fileInfo = list.at(i);
			if(fileInfo.isDir())
			{
				sizex += calcSize(fileInfo.absoluteFilePath());
			}
			else
				sizex += fileInfo.size();

		}
	}
	return sizex;
}

void Copier::update(qint64 pos)
{
	_window->updateProgressBar(_src, pos);
}

int Copier::countFile(const QString & path, bool b)
{
	int suma = 0;
	QDir dir(path);
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	if(!dir.exists()) {
		return 1;
	}
	QFileInfoList sList = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

	foreach(QFileInfo ruta, sList){
		if(ruta.isDir()){
			suma += countFile(ruta.path() + "/" + ruta.completeBaseName()+"/");
		}
		suma++;
	}
	return suma;
}

