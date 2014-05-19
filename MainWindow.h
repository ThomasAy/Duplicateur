/**
* Copyright (C) 2012-2014 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include  <QTime>



#include "Copier.h"
#include "Progression.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

protected :
	bool eventFilter(QObject *sender, QEvent *event);
	void dropEvent(QDropEvent *event);
	void dragEnterEvent(QDragEnterEvent *e);
private slots:
	void on_pushButton_clicked();

	void on_finnish();

	void on_usbDrives_itemActivated(QListWidgetItem *item);

	void on_pb_Clear_clicked();

	void on_pb_Select_clicked();

	void errorString(QString);

	void on_pushButton_2_clicked();

private:
	Ui::MainWindow *ui;
	QTime _t;
	int _nbThreads;

	Progression _p;
};

#endif // MAINWINDOW_H
