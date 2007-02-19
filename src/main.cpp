#include <QApplication>

#include "algebra.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	QAlgebra main(NULL);
	main.show();
	return app.exec();
}
