#pragma once



#include <qwt_plot.h>
#include <qlist.h>
#include <qwt_plot_spectrogram.h>

#include  "SlsQt2DZoomer.h"
#include  "SlsQt2DHist.h"


class  QwtPlotPanner;
class  QwtScaleWidget;
class  QwtLinearColorMap;


class SlsQt2DPlot: public QwtPlot{
    Q_OBJECT

public:
    SlsQt2DPlot(QWidget * = NULL);

    void SetTitle(QString title);
    void SetXTitle(QString title);
    void SetYTitle(QString title);
    void SetZTitle(QString title);
    void SetTitleFont(const QFont& f);
    void SetXFont(const QFont& f);
    void SetYFont(const QFont& f);
    void SetZFont(const QFont& f);

    void UnZoom(bool replot=true);
    void SetZoom(double xmin,double ymin,double x_width,double y_width);
    void DisableZoom(bool disable);
    void EnableXAutoScaling() {setAxisAutoScale(QwtPlot::xBottom, true);};
    void EnableYAutoScaling() {setAxisAutoScale(QwtPlot::yLeft, true);};
    void SetXMinMax(double min,double max){setAxisScale(QwtPlot::xBottom,min,max);};
    void SetYMinMax(double min,double max){setAxisScale(QwtPlot::yLeft,min,max);};
    double GetXMinimum(){return hist->GetXMin();};
    double GetXMaximum(){return hist->GetXMax();};
    double GetYMinimum(){return hist->GetYMin();};
    double GetYMaximum(){return hist->GetYMax();};
    double GetZMinimum(){ return hist->GetMinimum();}
    double GetZMaximum(){ return hist->GetMaximum();}
    void SetZMinMax(double zmin=0,double zmax=-1);
    void SetZMinimumToFirstGreaterThanZero(){hist->SetMinimumToFirstGreaterThanZero();}
    double GetZMean()   { return hist->GetMean();}

    void   SetData(int nbinsx, double xmin, double xmax, int nbinsy,double ymin, double ymax,double *d,double zmin=0, double zmax=-1){
      hist->SetData(nbinsx,xmin,xmax,nbinsy,ymin,ymax,d,zmin,zmax);
    }

    double* GetDataPtr()                        {return hist->GetDataPtr();}
    int GetBinIndex(int bx,int by)          {return hist->GetBinIndex(bx,by);}
    int FindBinIndex(double x,double y)     {return hist->FindBinIndex(x,y);}
    void SetBinValue(int bx,int by,double v) { hist->SetBinValue(bx,by,v);}
    double GetBinValue(int bx,int by)          {return hist->GetBinValue(bx,by);} 
    void FillTestPlot(int i=0);
    void Update();

    void SetInterpolate(bool enable);
    void SetContour(bool enable);
    void SetLogz(bool enable, bool isMin, bool isMax, double min, double max);
    void SetZRange(bool isMin, bool isMax, double min, double max);
    void LogZ(bool on=1);
  
   

public slots:
    void showSpectrogram(bool on);


private:
    void SetupZoom();
    void SetupColorMap();
    QwtLinearColorMap* myColourMap(QVector<double> colourStops);
    QwtLinearColorMap* myColourMap(int log=0);

    QwtPlotSpectrogram *d_spectrogram;
    SlsQt2DHist* hist;
    SlsQt2DZoomer* zoomer;
    QwtPlotPanner* panner;
    QwtScaleWidget *rightAxis;

    QwtLinearColorMap* colorMapLinearScale;
    QwtLinearColorMap* colorMapLogScale;
#if QWT_VERSION<0x060000
    QwtValueList* contourLevelsLinear;
    QwtValueList* contourLevelsLog;
#else 
    QList<double> contourLevelsLinear;
    QList<double> contourLevelsLog;
#endif
    bool disableZoom{false};
    int isLog;
};
