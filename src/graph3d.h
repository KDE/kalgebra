#ifndef Q3DGRAPH_H
#define Q3DGRAPH_H

#include <QGLWidget>

#include "analitza.h"

#define KEYRIGHT	1<<0
#define KEYLEFT		1<<1
#define KEYUP		1<<2
#define KEYDOWN		1<<3
#define KEYAVPAG	1<<4
#define KEYREPAG	1<<5
#define KEYS		1<<6
#define KEYW		1<<7
#define KEYQ		1<<8
#define KEYE		1<<9
#define LCLICK		1<<10
#define RCLICK		1<<11
#define MCLICK		1<<12


/**
@author Aleix Pol i Gonzalez
*/

class Calculate3D : public QThread
{
public:
	Calculate3D(QObject *p, const Analitza &na, double** poi, int f, int t, double m, double s) : 
		QThread(p), a(na), punts(poi), from(f), to(t), mida(m), step(s) {}
	void run();
	
private:
	Analitza a;
	double **punts;
	int from;
	int to;
	
	double mida;
	double step;
};

class Graph3D : public QGLWidget {
Q_OBJECT
public:
	enum Type { Dots=0, Lines=1, Solid=2 };
	
	Graph3D(QWidget *parent = 0);
	~Graph3D();

	virtual void initializeGL() ;
	virtual void resizeGL( int width, int height ) ;
	virtual void paintGL() ;
	void setFunc(QString Text);
	int setFuncMML(QString TextMML);
	void dibuixa_eixos();
	void setTransparency(bool tr) { trans = tr; glDraw(); }
	bool transparency() const { return trans; }
	QPixmap toPixmap();
// 	bool save(const KURL& url);
	
	void setMida(double max);
	void setStep(double res);
	void setZ(float coord_z);
	void setMethod(enum Type m);
public slots:
	void resetView();
private:
	void keyPressEvent(QKeyEvent *e);
	void keyReleaseEvent(QKeyEvent *e);
	void timeOut();
	void mousePressEvent(QMouseEvent *e); QPoint press;
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	int load();
	void mem();
	bool crea();
	void sendStatus(const QString& msg) { emit status(msg); }
	
	Expression func3d;
	double default_step;
	double default_size;
	double zoom;
	double **punts;
	float graus[3];
	float z;
	enum Type method;
	bool trans;
	bool tefunc;
	unsigned short keyspressed;
	
	int m_n;
signals:
	void status(const QString &msg);
};

#endif
