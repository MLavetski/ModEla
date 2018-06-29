#include "modela.h"
#include "ui_modela.h"
#include "QFileDialog"
#include "QMessageBox"

modela::modela(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::modela)
{
	ui->setupUi(this);
}

modela::~modela()
{
	delete ui;
}

void modela::on_openFileB_clicked()
{
	QString filePath = QFileDialog::getOpenFileName(this, "Open Data File", "", "txt file (*.txt)");
	if(filePath != "")
	{
		openedFiles.append(new odt(this, filePath));
		amOF++;
	}
	else
	{
		return;
	}
}

void modela::on_helpB_clicked()
{
	QMessageBox::information(this, "Help", "This programm calculates youngs modulus based on data from AFMs.");
}
