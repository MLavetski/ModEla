#include "modela.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	modela w;
	w.show();

	return a.exec();
}
