#pragma once

#include "ui_form_cloneplot.h"

class SlsQtH1D;
class SlsQt1DPlot;
class SlsQt2DPlot;

#include <QMainWindow>
#include <QString>

#include <iostream>
#include <string>

class qCloneWidget : public QMainWindow, private Ui::ClonePlotObject {
    Q_OBJECT

  public:
    qCloneWidget(QWidget *parent, SlsQt1DPlot *p1, SlsQt2DPlot *p2, SlsQt2DPlot *gp,
                 QString title, QString filePath, QString fileName,
                 int64_t aIndex, bool displayStats, QString min, QString max,
                 QString sum);

    ~qCloneWidget();

  private slots:
    void SavePlot();

  protected:
    void resizeEvent(QResizeEvent *event);
    
  private:
    void SetupWidgetWindow(QString title);
    void DisplayStats(bool enable, QString min, QString max, QString sum);

  private:
    int id;
    SlsQt1DPlot *plot1d{nullptr};
    SlsQt2DPlot *plot2d{nullptr};
    SlsQt2DPlot *gainplot2d{nullptr};
    QString filePath{"/"};
    QString fileName{"run"};
    int64_t acqIndex{0};

    static int NumClones;
};