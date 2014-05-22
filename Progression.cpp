/**
* Copyright (C) 2012-2014 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#include "Progression.h"
#include "ui_Progression.h"

#include <QFile>
#include <QDebug>

Progression::Progression(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Progression)
{
	ui->setupUi(this);
	vbl = new QVBoxLayout(this);
	this->setMinimumSize(800, 0);
}

Progression::~Progression()
{
	delete ui;
}

void Progression::addCopy(QString src, QString dest){
	QLabel * label = new QLabel(src, this);
	label->setObjectName(src);
	label->setMaximumSize(780, 30);
	label->setGeometry(label->x(), label->y(), 780, label->height());
	QProgressBar * prog = new QProgressBar(this);
	prog->setObjectName(src);
	prog->setMaximum(QFile(src).size());
	vbl->addWidget(label);
	vbl->addWidget(prog);
	ProgressBars[label->text()] = prog;
	qDebug() << label->text();
	this->adjustSize();
}

void Progression::updateProgressBar(QString file, qint64 pos, QString currentFile){
	ProgressBars[file]->setValue((int) pos);
	qDebug() << "Update" << vbl->count();
	if(currentFile != "")
	{
		for(int i = 0; i < vbl->count(); i++)
		{
			QLayoutItem* item = vbl->itemAt(i);
			QWidget* widget = item->widget();
			if (widget) {
				QLabel *label = qobject_cast<QLabel *>(widget);
				if(label->text().contains(file))
				{
					label->setText(file + " -> " + currentFile);
					label->adjustSize();
				}
			}
		}
	}
	this->adjustSize();
}

void Progression::incrementProgressBar(QString file, QString currentFile){
	//qDebug() << "The value was" << ProgressBars[file]->value();
	if(ProgressBars[file]->value() < ProgressBars[file]->maximum())
		ProgressBars[file]->setValue(ProgressBars[file]->value() + 1);
	//qDebug() << "The value is" << ProgressBars[file]->value();

	if(currentFile != "")
	{
		for(int i = 0; i < vbl->count(); i++)
		{
			QLayoutItem* item = vbl->itemAt(i);
			QWidget* widget = item->widget();
			if (widget) {
				QLabel *label = qobject_cast<QLabel *>(widget);
				if(label)
				{
					if(label->objectName() == file)
					{
						label->setText("Copy " + currentFile.split("/").last() + " to " + currentFile);
					}
				}
			}
		}
	}
	this->adjustSize();
}

void Progression::updateProgressBarMax(QString file, qint64 pos){
	//qDebug() << "The maximum is : " << pos;
	ProgressBars[file]->setMaximum((int) pos);
}

void Progression::onRemoveLabel(QString name)
{
	qDebug() << "trying to remove label and progress bar for :" << name;
	for(int i = 0; i < vbl->count(); i++)
	{
		QLayoutItem* item = vbl->itemAt(i);
		QWidget* widget = item->widget();
		if (widget) {
			QLabel *label = qobject_cast<QLabel *>(widget);

			if(label and label->objectName() == name )
			{
				label->hide();
				vbl->removeItem(item);
				qDebug() << "Remove Label";
				i--;
			}

			QProgressBar * pb = qobject_cast<QProgressBar *>(widget);
			if(pb and pb->objectName() == name)
			{
				pb->hide();
				vbl->removeItem(item);
				qDebug() << "Remove Progbar";
				i--;
			}

		}
	}
	this->adjustSize();
}

