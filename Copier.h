/**
* Copyright (C) 2012-2014 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#ifndef COPIER_H
#define COPIER_H

#include <QObject>
#include <QProgressBar>

class Copier : public QObject
{
	Q_OBJECT
public:
	explicit Copier(QObject *parent = 0);
	Copier(QString src, QString dest);

signals:
	void finished();
	void error(QString);


public slots:
	void process();

private:
	QString _src;
	QString _dest;


};

#endif // COPIER_H
