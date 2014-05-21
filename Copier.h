/**
* Copyright (C) 2012-2014 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#ifndef COPIER_H
#define COPIER_H

#include <QObject>

#include <Progression.h>

class Copier : public QObject
{
	Q_OBJECT
public:
	explicit Copier(QObject *parent = 0);
	Copier(QString src, QString dest, Progression * window);

	static bool rmDir(const QString &dirPath);
	static int calcSize(const QString &dirPath);

signals:
	void finished();
	void error(QString);


public slots:
	void process();

private slots:
	void update(qint64 pos);
private:
	QString _src;
	QString _dest;

	Progression * _window;
	bool cpDir(const QString &srcPath, const QString &dstPath);
	int countFile(const QString &path, bool countDirs = false);

	int _nbFiles;
};

#endif // COPIER_H
