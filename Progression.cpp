/**
* Copyright (C) 2012-2014 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#include "Progression.h"
#include "ui_Progression.h"

#include <QLabel>
#include <QProgressBar>
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
	qDebug() << "add a copy";
	QLabel label(dest, this);
	QProgressBar prog;
	prog.setMaximum(QFile(src).size());
	vbl->addWidget(&label);
	vbl->addWidget(&prog);
}
