#include "odt.h"
#include "ui_odt.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QPainter>
#include <QFileDialog>

odt::odt(QWidget *parent, QString inName) :
	QMainWindow(parent),
	ui(new Ui::odt),
	startPointer(nullptr),
	stopPointer(nullptr)
{
	filename = inName;
	ui->setupUi(this);
	startPointer = new QCPItemLine(ui->label);
	stopPointer = new QCPItemLine(ui->label);
	if(openFile())
	{
		this->show();
		drawData();
		proposePoints();
	}
	else
		this->hide();
}

odt::~odt()
{
	delete ui;
}

bool odt::openFile()
{
	QFile inFile(filename);
	if(!inFile.open(QIODevice::ReadOnly))
	{
		QMessageBox::critical(this, tr("Error opening file"), tr("An unknown error occured while trying"
																 " to open a file"));
		return false;
	}
	readFile = new QTextStream(&inFile);
	QString testForCorrectFileType;
	readFile->readLineInto(&testForCorrectFileType);
	if(testForCorrectFileType == "Spectroscopy data")
	{
		readNT();
	}
	else if(testForCorrectFileType == "    nm         	mV")
	{
		readFS();
	}
	else
	{
		QMessageBox::critical(this, tr("Wrong file type"), tr("Opened file doesn't contain data"));
		return false;
	}
	QFileInfo info(filename);
	this->setWindowTitle(info.fileName());
	return true;
}

void odt::readNT()
{
	QRegExp rx("-?\\d+\\.\\d+");
	//0-1 -
	//any amount of numbers
	//any symbol(comma or point it is)
	//any amount of numbers
	int rowsRead = 0;
	while (true)
	{
		QString currentLine;
		readFile->readLineInto(&currentLine);
		if(currentLine==nullptr)
		{
			if(rowsRead>2)
				break;
		}
		QStringList numbersInText;
		int posF = 0;
		while((posF = rx.indexIn(currentLine, posF)) != -1)
		{
			posF+=rx.matchedLength();
			numbersInText<<rx.capturedTexts();
		}
		if(rowsRead>2)
		{
			z.append(numbersInText.at(0).toDouble()*(-1));
			defl.append(numbersInText.at(1).toDouble());
		}
		rowsRead++;
	}
}

void odt::readFS()
{
	QRegExp rx("-?\\d+\\.\\d+(?:e[-?+?]\\d+)?");
	//0-1 -
	//any amount of numbers
	//any symbol(comma or point it is)
	//any amount of numbers
	//possible scientific notation(e, + or - and any amount of numbers)
	while (true)
	{
		QString currentLine;
		readFile->readLineInto(&currentLine);
		if(currentLine==nullptr)
			break;
		QStringList numbersInText;
		int posF = 0;
		while((posF = rx.indexIn(currentLine, posF)) != -1)
		{
			posF+=rx.matchedLength();
			numbersInText<<rx.capturedTexts();
		}
		z.append(numbersInText.at(0).toDouble()*(-1));
		defl.append(numbersInText.at(1).toDouble());
	}
	//"reflect" fastscan graph on y axis.
	std::reverse(defl.begin(), defl.end());
}

void odt::drawData()
{
	int amPoints=z.size();
	max=defl[0]; min=defl[0];
	for(int i=0;i<amPoints;i++)
	{
		if(max<defl[i])
			max=defl[i];
		if(min>defl[i])
			min=defl[i];
	}
	ui->startPSlider->setMaximum(amPoints-1);
	ui->stopPSlider->setMaximum(amPoints-1);
	auto* plotData = ui->label;
	plotData->addGraph();
	plotData->graph(0)->addData(z,defl);
	plotData->xAxis->setLabel("z, nm");
	plotData->yAxis->setLabel("defl");
	plotData->rescaleAxes();
	plotData->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iMultiSelect);
	plotData->replot();
}

void odt::updateSelectedP()
{
	//update text info
	int startP = ui->startPSlider->value(), endP = ui->stopPSlider->value();
	QString generatedInfo = "Start point: x:"+QString::number(z[startP])+"; Deflection:"+QString::number(defl[startP])
			+";\tEnd point: x:"+QString::number(z[endP])+"; Deflection:"+QString::number(defl[endP])+".";
	ui->infoLabel->setText(generatedInfo);
	//draw arrows pointing to selected points
	startPointer->start->setCoords(z[startP], defl[startP]+1);
	startPointer->end->setCoords(z[startP], defl[startP]);
	startPointer->setHead(QCPLineEnding::esSpikeArrow);
	stopPointer->start->setCoords(z[endP], defl[endP]+1);
	stopPointer->end->setCoords(z[endP], defl[endP]);
	stopPointer->setHead(QCPLineEnding::esSpikeArrow);
	ui->label->replot();
}

void odt::on_startPSlider_valueChanged()
{//redirection to prepared function since designer is wonky
	this->updateSelectedP();
}

void odt::on_stopPSlider_valueChanged()
{
	this->updateSelectedP();
}

void odt::proposePoints()
{
	bool minPassed = false, startFound = false;
	for(int i=0;i<defl.size();i++)
	{
		if(defl[i]<=min)
		{
			minPassed = true;
		}
		if(minPassed && defl[i]>0 && !startFound)
		{
			ui->startPSlider->setValue(i);
			startFound = true;
		}
		if(startFound && (((max-defl[i])/(max-min))<0.1))
		{
			ui->stopPSlider->setValue(i);
			break;
		}
	}
}

void odt::on_tipSpnBox_valueChanged(double arg1)
{
	R=arg1;
}

void odt::on_hardSpinBox_valueChanged(double arg1)
{
	k=arg1;
}

void odt::on_poissonSpinBox_valueChanged(double arg1)
{
	v=arg1;
}

void odt::on_calRatSpinBox_valueChanged(double arg1)
{
	calRat=arg1;
}

void odt::on_calcMod_clicked()
{
	ui->saveB->setEnabled(false);
	E.clear();
	depth.clear();
	int startP = ui->startPSlider->value(), stopP = ui->stopPSlider->value();
	for(int i=startP;i<stopP;i++)
	{
		tmpZ = (z[i]-z[startP-1]);
		trueDefl = ((defl[i]+((-1)*defl[startP-1]))/calRat);
		tmpDepth = tmpZ-trueDefl;
		tmpE = (3*(1-(v*v))*k*trueDefl*pow(10,-9))/(4*sqrt(R*pow(10,-9))*pow((tmpDepth*pow(10,-9)),1.5));
		E.append(tmpE);
		depth.append(tmpDepth);
	}
	avrE=0;
	for(int i=startP;i<stopP;i++)
	{
		if(!qIsNaN(E[i-startP]))
			avrE+=E[i-startP];
	}
	avrE/=(stopP-startP);
	auto* plot = ui->plotE;
	plot->removeGraph(0);
	plot->addGraph();
	plot->graph(0)->addData(depth,E);
	plot->xAxis->setLabel("depth, nm");
	plot->yAxis->setLabel("E, pa");
	plot->rescaleAxes();
	plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
	plot->replot();
	ui->saveB->setEnabled(true);
	QMessageBox::information(this,"Done", ("Done. Average E="+QString::number(avrE)));
}

void odt::on_saveB_clicked()
{
	int startP = ui->startPSlider->value(), stopP = ui->stopPSlider->value();
	QFileInfo info(filename);
	QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save File"), info.filePath().split(".").at(0)+"_modulus.txt",
														tr("Text file (*.txt)"));
	QFile out(saveFileName);
	if(!out.open(QIODevice::WriteOnly))
		return;
	QTextStream outText(&out);
	outText << "Name: " + filename;
	outText << "\nDepth, nm\tE, Pa";
	for(int i=startP;i<stopP;i++)
	{
		outText << qPrintable("\n" + QString::number(depth[i-startP]) + "\t" + QString::number(E[i-startP]));
	}
	outText << qPrintable("\nAverage:\t" + QString::number(avrE));
	outText.flush();
	out.close();
	QMessageBox::information(this,"Done", ("Done. File was saved."));
}
