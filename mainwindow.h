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

    boost::filesystem::path OutputDirectory;

    cv::Mat ImIn;
    cv::Mat ImShow;

    int maxX;
    int maxY;

    int tileShape;
    int maxTileX;
    int maxTileY;
    int shiftTileX;
    int shiftTileY;
    int offsetTileX;
    int offsetTileY;

    int tileLineThickness;

    int *TilesX;
    int *TilesY;

    std::vector<std::string> NamesVector;


    boost::filesystem::path FileToOpen2;
    boost::filesystem::path CurrentDir2;
    boost::filesystem::path InputDirectory2;
    boost::filesystem::path ImFileName2;

    boost::filesystem::path OutputDirectory2;

    cv::Mat ImIn2;
    cv::Mat ImShow2;

    int maxX2;
    int maxY2;

    int tileShape2;
    int maxTileX2;
    int maxTileY2;
    int shiftTileX2;
    int shiftTileY2;
    int offsetTileX2;
    int offsetTileY2;


    int *TilesX2;
    int *TilesY2;

    std::vector<std::string> NamesVector2;


    //float
    FileParams GetDirectionData(boost::filesystem::path FileToOpen);

    void ProcessImage();

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

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
