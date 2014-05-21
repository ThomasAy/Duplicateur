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
}

Progression::~Progression()
{
	delete ui;
}

void Progression::addCopy(QString src, QString dest){
	QLabel * label = new QLabel(src, this);
	QProgressBar * prog = new QProgressBar(this);
	prog->setMaximum(QFile(src).size());
	vbl->addWidget(label);
	vbl->addWidget(prog);
	ProgressBars[label->text()] = prog;
	qDebug() << label->text();
}

void Progression::updateProgressBar(QString file, qint64 pos){
	ProgressBars[file]->setValue((int) pos);
}

void Progression::incrementProgressBar(QString file){
	//qDebug() << "The value was" << ProgressBars[file]->value();
	if(ProgressBars[file]->value() < ProgressBars[file]->maximum())
		ProgressBars[file]->setValue(ProgressBars[file]->value() + 1);
	//qDebug() << "The value is" << ProgressBars[file]->value();
}

void Progression::updateProgressBarMax(QString file, qint64 pos){
	//qDebug() << "The maximum is : " << pos;
	ProgressBars[file]->setMaximum((int) pos);
}

