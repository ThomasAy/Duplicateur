/**
* Copyright (C) 2012-2014 Phonations
* License: http://www.gnu.org/licenses/gpl.html GPL version 2 or higher
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include  <QTime>
#include <QTimer>



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
	bool eventFilter(QObject *, QEvent *event);
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

	void on_pb_Eject_clicked();

	void refreshList();

	void on_pb_browse_clicked();

	void on_pb_browse_folders_clicked();

	void on_radio_folder_yes_clicked();

	void on_radio_folder_no_clicked();

	void on_actionAbout_triggered();

private:
	Ui::MainWindow *ui;
	QTime _t;
	int _nbThreads;

	Progression _p;
	QTimer _tRefresh;

};

#endif // MAINWINDOW_H
