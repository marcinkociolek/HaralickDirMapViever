#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <boost/filesystem.hpp>

#include <opencv2/core/core.hpp>

#include <QMainWindow>

#include "tileparams.h"

//using namespace boost::filesystem;
//using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    boost::filesystem::path FileToOpen;
    boost::filesystem::path CurrentDir;
    boost::filesystem::path InputDirectory;
    boost::filesystem::path ImFileName;

    boost::filesystem::path StartDir;

    boost::filesystem::path FileToOpen2;
    //    boost::filesystem::path CurrentDir2;
    boost::filesystem::path InputDirectory2;
    //    boost::filesystem::path ImFileName2;

    boost::filesystem::path OutputDirectory;

    cv::Mat ImIn;
    cv::Mat ImIn2;
    cv::Mat ImShow;

    FileParams FilePar1;
    FileParams FilePar2;

    bool sudocolor;
    bool showShape;
    bool showLine;
    bool showFirstImage;
    bool showSecondImage;
    bool showTwoImages;
    float minIm;
    float maxIm;
    float minIm2;
    float maxIm2;
    int tileLineThickness;
    int featNr;
    float meanIntensityTreshold;
    float meanIntensityTreshold2;
    double lineLength;
    int imposedLineThickness;

    int zOffset;
    int zFrame;

    //    int maxX;
//    int maxY;

//    int tileShape;
//    int maxTileX;
//    int maxTileY;
//    int shiftTileX;
//    int shiftTileY;
//    int offsetTileX;
//    int offsetTileY;

//    int tileLineThickness;

//    int *TilesX;
//    int *TilesY;

//    std::vector<std::string> NamesVector;




//    boost::filesystem::path OutputDirectory2;

//    cv::Mat ImIn2;
//    cv::Mat ImShow2;

//    int maxX2;
//    int maxY2;

//    int tileShape2;
//    int maxTileX2;
//    int maxTileY2;
//    int shiftTileX2;
//    int shiftTileY2;
//    int offsetTileX2;
//    int offsetTileY2;


//    int *TilesX2;
 //   int *TilesY2;

//    std::vector<std::string> NamesVector2;


    //float
    FileParams MainWindow::GetDirectionData(boost::filesystem::path FileToOpen);


    void MainWindow::ShowImage(cv::Mat Im, FileParams Params,
                               bool sudocolor,
                               bool showShape,
                               bool showLine,
                               float minIm,
                               float maxIm,
                               int tileLineThickness,
                               int featNr,
                               float meanIntensityTreshold,
                               double lineLength,
                               int imposedLineThickness,
                               std::string ShowWindowName);

    void MainWindow::Show2Image(cv::Mat Im, cv::Mat Im2, FileParams Params, FileParams Params2,
                               bool sudocolor,
                               bool showShape,
                               bool showLine,
                               float minIm, float maxIm, float minIm2, float maxIm2,
                               int tileLineThickness,
                               int featNr,
                               float meanIntensityTreshold, float meanIntensityTreshold2,
                               double lineLength,
                               int imposedLineThickness);

private slots:
    void on_pushButton_clicked();

    void on_FileListWidget_currentTextChanged(const QString &currentText);

    void on_checkBoxShowSape_toggled(bool checked);

    void on_checkBoxShowLine_toggled(bool checked);

    void on_checkBoxShowSudoColor_toggled(bool checked);

    void on_spinBoxImposedShapeThickness_valueChanged(int arg1);

    void on_spinBoxImposedLineThickness_valueChanged(int arg1);

    void on_spinBoxLineLength_valueChanged(int arg1);

    void on_spinBoxFeatureToShow_valueChanged(int arg1);

    void on_pushButtonChoseOutDir_clicked();

    void on_pushButtonSaveOut_pressed();

    void on_doubleSpinBoxImMin_valueChanged(double arg1);

    void on_doubleSpinBoxImMax_valueChanged(double arg1);

    void on_doubleSpinBoxProcTresh_valueChanged(double arg1);

    void on_pushButton2_clicked();

    void on_File2ListWidget_currentTextChanged(const QString &currentText);

    void on_doubleSpinBoxImMin2_valueChanged(double arg1);

    void on_doubleSpinBoxImMax2_valueChanged(double arg1);

    void on_doubleSpinBoxProcTresh2_valueChanged(double arg1);

    void on_checkBoxShowSecondIm_toggled(bool checked);

    void on_checkBoxShowFirstIm_toggled(bool checked);

    void on_checkBoxShowTwoIm_toggled(bool checked);

    void on_pushButtonPlus_clicked();

    void on_spinBoxZOffset_valueChanged(int arg1);

    void on_pushButtonMinus_clicked();

    void on_pushButtonCreateOut_clicked();


    void on_pushCreateHist_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
