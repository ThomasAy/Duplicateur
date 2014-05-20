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

	static bool rmDir(const QString &dirPath);

signals:
	void finished();
	void error(QString);


public slots:
	void process();

private:
	QString _src;
	QString _dest;


	bool cpDir(const QString &srcPath, const QString &dstPath);
};

#endif // COPIER_H
