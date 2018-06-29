#ifndef ODT_H
#define ODT_H

#include <QMainWindow>
#include <QTextStream>

namespace Ui {
class odt;
}

class odt : public QMainWindow
{
	Q_OBJECT

public:
	explicit odt(QWidget *parent = 0, QString inName = "");
	~odt();

private slots:
	void on_startPSlider_valueChanged();

	void on_stopPSlider_valueChanged();

	void on_tipSpnBox_valueChanged(double arg1);

	void on_hardSpinBox_valueChanged(double arg1);

	void on_poissonSpinBox_valueChanged(double arg1);

	void on_calRatSpinBox_valueChanged(double arg1);

	void on_calcMod_clicked();

	void on_saveB_clicked();

private:
	Ui::odt *ui;
	QString filename;
	QTextStream *readFile;
	QVector<double> z, defl, E, depth;
	QImage graph;
	double max, min, sizeMult=0.8, R=10, k=27, v=0.3, calRat=1.25;
	double tmpZ, trueDefl, tmpDepth, tmpE, avrE=0;
	bool openFile();
	void readNT();
	void readFS();
	void drawData();
	void updateSelectedP();
	void proposePoints();
};

#endif // ODT_H
