/**
* Copyright (C) 2012-2014 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#ifndef PROGRESSION_H
#define PROGRESSION_H

#include <QWidget>
#include <QVBoxLayout>
#include <QMap>

#include <QLabel>
#include <QProgressBar>

namespace Ui {
class Progression;
}

class Progression : public QWidget
{
	Q_OBJECT

public:
	explicit Progression(QWidget *parent = 0);
	~Progression();

	void addCopy(QString src, QString dest);

	void updateProgressBarMax(QString file, qint64 pos);

	void incrementProgressBar(QString file);
public slots:
	void updateProgressBar(QString file, qint64 pos);
private:
	Ui::Progression *ui;
	QVBoxLayout *vbl;
	QMap<QString, QProgressBar *> ProgressBars;
};

#endif // PROGRESSION_H
