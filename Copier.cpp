/**
* Copyright (C) 2012-2014 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#include "Copier.h"
#include <QFile>
#include <QDebug>
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
	if(QFile(_dest).exists())
	{
		emit error("The file" + _dest + " already exist!");
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
