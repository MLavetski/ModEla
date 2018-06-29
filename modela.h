#ifndef MODELA_H
#define MODELA_H

#include <QMainWindow>
#include <odt.h>

namespace Ui {
class modela;
}

class modela : public QMainWindow
{
	Q_OBJECT

public:
	explicit modela(QWidget *parent = 0);
	~modela();

private slots:
	void on_openFileB_clicked();

	void on_helpB_clicked();

private:
	Ui::modela *ui;
	QVector<odt *> openedFiles;
	quint16 amOF = 0;
};

#endif // MODELA_H
