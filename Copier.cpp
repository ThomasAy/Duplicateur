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

Copier::Copier(QObject *parent) :
	QObject(parent)
{
}

Copier::Copier(QString src, QString dest)
{
	_src = src;
	_dest = dest;
}

void Copier::process(){
	if(QFile(_dest).exists() && _src.at(_src.count() - 1) != '/')
	{
		emit error("The file" + _dest + " already exist!");
	}
	else if(_src.at(_src.count() - 1) == '/')
	{
		cpDir(_src, _dest + "/" + _src.split("/").at(_src.split("/").count() - 2));
		qDebug() << _dest << "copy finnished";
		emit finished();
	}
	else
	{
		QFile fromFile(_src);
		QFile toFile(_dest);


		if(!fromFile.copy(toFile.fileName()))
		{
			emit error("An error occur during the copy of the file:" + _dest);
			emit finished();
		}


		qDebug() << _dest << "copy finnished";
		emit finished();
	}
}

bool Copier::cpDir(const QString &srcPath, const QString &dstPath)
{
	qDebug() << "copy from" << srcPath << "to" << dstPath;
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
