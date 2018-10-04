#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>


#include <chrono>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//#include "gradient.h"
#include "DispLib.h"
#include "StringFcLib.h"
#include "tileparams.h"
#include "dirdetectionparams.h"

#define PI 3.14159265

using namespace boost;
using namespace std;
using namespace boost::filesystem;
using namespace cv;

std::string OutputStringFromNumber(double number)
{
    if(number < -999.0)
        return " ";
    else
        return to_string(number);
}

double FindResultingDirection(int *DirectionHistogram)
{
    double longestResultingVector = 0;
    double resultingDirection = -1000;
    for(int startDir = 0; startDir<180; startDir++)
    {
        double a = 0.0;
        double b = 0.0;
        int stopDir = startDir + 180;

        for(int dir = startDir; dir < stopDir; dir++)
        {
            int normDir;
            if(dir < 180)
                normDir = dir;
            else
                normDir = dir - 180;
            double length = (double)DirectionHistogram[normDir];
            a += length * cos((double)dir/180.0*PI);
            b += length * sin((double)dir/180.0*PI);
        }
        double localLength = sqrt(a*a+b*b);
        if(longestResultingVector < localLength)
        {
            longestResultingVector = localLength;
            if(a == 0.0 && b == 0.0)
                resultingDirection = -1000.0;
            else
                resultingDirection = atan2(b,a)/PI*180.0;

        }
    }
    if(resultingDirection < 0.0)
        resultingDirection += 360.0;
    if (resultingDirection >= 180.0)
        resultingDirection -=180.0;
    if(resultingDirection < 0.0 || resultingDirection > 180.0)
        return -1000.0;

    return resultingDirection;
}
//----------------------------------------------------------------------------------
double FindSpread(int *DirectionHistogram, double resultingDir)
{
    if (resultingDir < -999.0)
        return -1000.0;

    int startDir =  (int)round(resultingDir) - 90;
    int stopDir = startDir + 180;
    double sum = 0;
    double count = 0;
    for(int dir = startDir; dir < stopDir; dir++)
    {
        int normDir;
        normDir = dir;
        if(normDir >= 180)
            normDir = dir - 180;
        if(normDir < 0)
            normDir = dir + 180;
        sum += (resultingDir - dir) * (resultingDir - dir) * DirectionHistogram[normDir];
        count += DirectionHistogram[normDir];
    }
    if(count != 0.0)
        return sqrt(sum/count);
    else
        return -2000.0;
}
//----------------------------------------------------------------------------------
string DirHistogramToString(int *DirectionHistogram)
{
    string Out = "Hist";
    for(int dir = 0; dir < 180; dir++)
    {
        Out += " \t";
        Out += to_string(DirectionHistogram[dir]);

    }
    for(int dir = 0; dir < 180; dir++)
    {
        Out += " \t";
        Out += to_string(DirectionHistogram[dir]);

    }
    return Out;
}
//----------------------------------------------------------------------------------
string DirDifferenceHistogramToString(int *DirectionDiffHistogram)
{
    string Out = "Hist";
    for(int dir = 0; dir <= 90; dir++)
    {
        Out += " \t";
        Out += to_string(DirectionDiffHistogram[dir]);

    }
    return Out;
}
//----------------------------------------------------------------------------------
double MeanOfDirDiffHistogram(int *DirectionDiffHistogram)
{
    int sum = 0;
    int count = 0;
    for(int dir = 0; dir <= 90; dir++)
    {
        sum += dir * DirectionDiffHistogram[dir];
        count += DirectionDiffHistogram[dir];
    }
    if (count != 0)
        return ((double)sum)/(double)count;
    else
        return -1000.0;
}
//----------------------------------------------------------------------------------
double StdOfDirDiffHistogram(int *DirectionDiffHistogram, double mean)
{
    double sum = 0;
    int count = 0;
    for(int dir = 0; dir <= 90; dir++)
    {
        double diff = ((double)dir - mean);
        sum += diff * diff * (double)DirectionDiffHistogram[dir];
        count += DirectionDiffHistogram[dir];
    }
    if (count != 0)
        return sqrt((sum)/(double)count);
    else
        return -1000.0;

}

//----------------------------------------------------------------------------------
void ShowShape(Mat ImShow, int x,int y, int tileShape, int tileSize, int tileLineThickness)
{
    switch (tileShape)
    {
    case 1:
        ellipse(ImShow, Point(x, y),
        Size(tileSize / 2, tileSize / 2), 0.0, 0.0, 360.0,
        Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
        break;
    case 2:

    {
        int edgeLength = tileSize / 2;
        Point vertice0(x - edgeLength / 2, y - (int)((float)edgeLength * 0.8660254));
        Point vertice1(x + edgeLength - edgeLength / 2, y - (int)((float)edgeLength * 0.8660254));
        Point vertice2(x + edgeLength, y);
        Point vertice3(x + edgeLength - edgeLength / 2, y + (int)((float)edgeLength * 0.8660254));
        Point vertice4(x - edgeLength / 2, y + (int)((float)edgeLength * 0.8660254));
        Point vertice5(x - edgeLength, y);

        line(ImShow, vertice0, vertice1, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
        line(ImShow, vertice1, vertice2, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
        line(ImShow, vertice2, vertice3, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
        line(ImShow, vertice3, vertice4, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
        line(ImShow, vertice4, vertice5, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
        line(ImShow, vertice5, vertice0, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
    }
        break;
    default:
        rectangle(ImShow, Point(x - tileSize / 2, y - tileSize / 2),
            Point(x - tileSize / 2 + tileSize, y - tileSize / 2 + tileSize / 2),
            Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
        break;
    }
}
//----------------------------------------------------------------------------------
void ShowShape(Mat ImShow, int x,int y, int tileShape, int tileSize, int tileLineThickness, Scalar LineColor)
{
    switch (tileShape)
    {
    case 1:
        ellipse(ImShow, Point(x, y),
        Size(tileSize / 2, tileSize / 2), 0.0, 0.0, 360.0,
        LineColor, tileLineThickness);
        break;
    case 2:

    {
        int edgeLength = tileSize / 2;
        Point vertice0(x - edgeLength / 2, y - (int)((float)edgeLength * 0.8660254));
        Point vertice1(x + edgeLength - edgeLength / 2, y - (int)((float)edgeLength * 0.8660254));
        Point vertice2(x + edgeLength, y);
        Point vertice3(x + edgeLength - edgeLength / 2, y + (int)((float)edgeLength * 0.8660254));
        Point vertice4(x - edgeLength / 2, y + (int)((float)edgeLength * 0.8660254));
        Point vertice5(x - edgeLength, y);

        line(ImShow, vertice0, vertice1, LineColor, tileLineThickness);
        line(ImShow, vertice1, vertice2, LineColor, tileLineThickness);
        line(ImShow, vertice2, vertice3, LineColor, tileLineThickness);
        line(ImShow, vertice3, vertice4, LineColor, tileLineThickness);
        line(ImShow, vertice4, vertice5, LineColor, tileLineThickness);
        line(ImShow, vertice5, vertice0, LineColor, tileLineThickness);
    }
        break;
    default:
        rectangle(ImShow, Point(x - tileSize / 2, y - tileSize / 2),
            Point(x - tileSize / 2 + tileSize, y - tileSize / 2 + tileSize / 2),
            LineColor, tileLineThickness);
        break;
    }
}
//--------------------------------------------------------------------------------
 void CreateDispalyWindows(bool showIm1, bool showIm2, bool showIm1and2, bool showImComb)
 {
     int flag = WINDOW_AUTOSIZE;//WINDOW_NORMAL;
     if (showIm1)
     {
         namedWindow("In File 1", flag);
     }
     else
         destroyWindow("In File 1");

     if (showIm2)
     {
         namedWindow("In File 2", flag);
     }
     else
         destroyWindow("In File 2");

     if (showIm1and2)
     {
         namedWindow("Two images", flag);
     }
     else
         destroyWindow("Two images");

     if (showImComb)
     {
         namedWindow("Combination", flag);
     }
     else
         destroyWindow("Combination");

 }
//--------------------------------------------------------------------------------
void ResizeImages(int scale, int orgSizeY, int orgSizeX)
{
    return;
    if (scale < 1)
        return;
    if (orgSizeX < 50)
        return;
    if (orgSizeY < 50)
        return;

    resizeWindow("In File 1", orgSizeX / scale, orgSizeY / scale);
    resizeWindow("In File 2", orgSizeX / scale, orgSizeY / scale);
    resizeWindow("Two images", orgSizeX / scale, orgSizeY / scale * 2);
}
//--------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sudocolor = ui->checkBoxShowSudoColor->checkState();
    showShape = ui->checkBoxShowSape->checkState();
    showLine = ui->checkBoxShowLine->checkState();
    minIm = ui->doubleSpinBoxImMin->value();
    maxIm = ui->doubleSpinBoxImMax->value();
    tileLineThickness = ui->spinBoxImposedShapeThickness->value();
    featNr = 0;//featNr = ui->spinBoxFeatureToShow->value();
    meanIntensityTreshold = ui->doubleSpinBoxProcTresh->value();
    lineLength = ui->spinBoxLineLength->value();
    imposedLineThickness = ui->spinBoxImposedLineThickness->value();
    showFirstImage = ui->checkBoxShowFirstIm->checkState();
    showSecondImage = ui->checkBoxShowSecondIm->checkState();
    showTwoImages = ui->checkBoxShowTwoIm->checkState();
    minIm2 = ui->doubleSpinBoxImMin2->value();
    maxIm2 = ui->doubleSpinBoxImMax2->value();
    minIm3 = ui->doubleSpinBoxImMin3->value();
    maxIm3 = ui->doubleSpinBoxImMax3->value();
    meanIntensityTreshold2 = ui->doubleSpinBoxProcTresh2->value();
    zOffset = 0; // = ui->spinBoxZOffset->value();
    zFrame = 0;
    showVectIm1 = ui->checkBoxShowVectIm1->checkState();
    showVectIm2 = ui->checkBoxShowVectIm2->checkState();
    showVectIm3 = ui->checkBoxShowVectIm3->checkState();
    showImageCombination = ui->checkBoxShowImageCombination->checkState();
    vectSliceOffset = ui->spinBoxImOffsetVect->value();

    InputDirectory = "E:/ActinCalceinData/Actin/Direction/";

    intensityThresholdIm1 = ui->spinBoxIntensityThreshold->value();
    intensityThresholdIm2 = ui->spinBoxIntensityThreshold2->value();

    displayScale = ui->spinBoxDisplayScale->value();


    ui->comboBoxImageScale->addItem("Images scale x 4");
    ui->comboBoxImageScale->addItem("Images scale x 2");
    ui->comboBoxImageScale->addItem("Images scale x 1");
    ui->comboBoxImageScale->addItem("Images scale x 1/2");
    ui->comboBoxImageScale->addItem("Images scale x 1/4");
    ui->comboBoxImageScale->setCurrentIndex(2);
    imageScale = 1.0;

    CreateDispalyWindows(showFirstImage, showSecondImage, showTwoImages, showImageCombination);


}

MainWindow::~MainWindow()
{
    delete ui;
}
//----------------------------------------------------------------------------------------------------------------
/*
FileParams MainWindow::GetDirectionData(path FileToOpen)
{
    FileParams LocalParams;
    //check if file exists
    if (!exists(FileToOpen))
        return LocalParams;
    std::ifstream inFile1(FileToOpen.string());
    if (!inFile1.is_open())
    {
        QMessageBox msgBox;
        msgBox.setText((FileToOpen.filename().string() + " File not exists" ).c_str());
        msgBox.exec();
        return LocalParams;
    }
    // ------------------read params from file-----------------------------

    string Line1,Line2;

    //read input directory
    bool inputDirFound = 0;
    while (inFile1.good())
    {
        getline(inFile1, Line1,'\t');
        getline(inFile1, Line2);
        regex LinePattern("Input Directory 1:");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            inputDirFound = 1;
            break;
        }
    }
    LocalParams.ImFileName = Line2;
    //path ImFileName(Line2);

    // read tile shape
    while (inFile1.good())
    {
        getline(inFile1, Line1);
        regex LinePattern("Tile Shape:.+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            break;
        }
    }

    LocalParams.tileShape = stoi(Line1.substr(12,1));

    //readTileSizeX
    while (inFile1.good())
    {
        getline(inFile1, Line1);

        regex LinePattern("Tile size x.+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            break;
        }
    }
    LocalParams.maxTileX = stoi(Line1.substr(13));
    //readTileSizeY
    while (inFile1.good())
    {
        getline(inFile1, Line1);

        regex LinePattern("Tile size y.+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            break;
        }
    }
    LocalParams.maxTileY = stoi(Line1.substr(13));
    //read shiftTileX
    while (inFile1.good())
    {
        getline(inFile1, Line1);

        regex LinePattern("Tile shift x:.+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            break;
        }
    }
    LocalParams.shiftTileX = stoi(Line1.substr(14));
    //read shiftTileY
    while (inFile1.good())
    {
        getline(inFile1, Line1);

        regex LinePattern("Tile shift y:.+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            break;
        }
    }
    LocalParams.shiftTileY = stoi(Line1.substr(14));

    //read offsetTileX
    while (inFile1.good())
    {
        getline(inFile1, Line1);

        regex LinePattern("Tile offset x:.+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            break;
        }
    }
    LocalParams.offsetTileX = stoi(Line1.substr(15));

    //read offsetTileY
    while (inFile1.good())
    {
        getline(inFile1, Line1);

        regex LinePattern("Tile offset y:.+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            break;
        }
    }
    LocalParams.offsetTileY = stoi(Line1.substr(15));


    // read input file name
    while (inFile1.good())
    {
        getline(inFile1, Line1);

        regex LinePattern("In file - .+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            break;
        }
    }
    LocalParams.ImFileName.append(Line1.substr(10));

    // read size of data vector
    while (inFile1.good())
    {
        getline(inFile1, Line1);

        regex LinePattern("Tile Y.+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            break;
        }
    }
    LocalParams.ValueCount = 0;
    size_t stringPos = 0;
    while(1)
    {
        stringPos = Line1.find("\t",stringPos);
        LocalParams.ValueCount++;
        if(stringPos != string::npos)
            break;
        stringPos++;
    }
    // read feature names
    std::stringstream InStringStream(Line1);

    LocalParams.NamesVector.empty();
    char FeatName[256];
    while(InStringStream.good())
    {
        InStringStream.getline(FeatName,250,'\t');
        LocalParams.NamesVector.push_back(FeatName);
    }

    //list<int> TilesParams;
    LocalParams.ParamsVect.empty();
//read directionalities
    while(inFile1.good())
    {
        TileParams params;
        getline(inFile1,Line1);
        params.FromString(Line1);
        if(params.tileX > -1 && params.tileY > -1)
            LocalParams.ParamsVect.push_back(params);
    }

    inFile1.close();
    return LocalParams;
}
*/
//----------------------------------------------------------------------------------------------------------------
FileParams  MainWindow::GetDirectionData(path FileToOpen)
{
    FileParams LocalParams;
    //check if file exists
    if (!exists(FileToOpen))
        return LocalParams;
    std::ifstream inFile1(FileToOpen.string());
    if (!inFile1.is_open())
    {
        QMessageBox msgBox;
        msgBox.setText((FileToOpen.filename().string() + " File not exists" ).c_str());
        msgBox.exec();
        return LocalParams;
    }
    // ------------------read params from file-----------------------------

    string Line;

    //read input directory
    bool inputDirFound = 0;
    while (inFile1.good())
    {
        getline(inFile1, Line);
        regex LinePattern("Input Directory.+");
        if (regex_match(Line.c_str(), LinePattern))
        {
            inputDirFound = 1;
            break;
        }
    }
    if(inputDirFound)
        LocalParams.ImFolderName = Line.substr(19);
    else
        return LocalParams;
    //path ImFileName(Line2);

    // read tile shape
    bool tileShapeFound = 0;
    while (inFile1.good())
    {
        getline(inFile1, Line);
        regex LinePattern("Tile Shape:.+");
        if (regex_match(Line.c_str(), LinePattern))
        {
            tileShapeFound = 1;
            break;
        }
    }
    if(tileShapeFound)
        LocalParams.tileShape = stoi(Line.substr(12,1));
    else
        return LocalParams;

    //readTileSizeX
    bool tileSizeFound = 0;
    while (inFile1.good())
    {
        getline(inFile1, Line);

        regex LinePattern("Tile width x:.+");
        if (regex_match(Line.c_str(), LinePattern))
        {
            tileSizeFound = 1;
            break;
        }
    }
    if(tileShapeFound)
            LocalParams.tileSize = stoi(Line.substr(13));
    else
        return LocalParams;

    // read input file name
    bool fileNameFound = 0;
    while (inFile1.good())
    {
        getline(inFile1, Line);

        regex LinePattern("File Name.+");
        if (regex_match(Line.c_str(), LinePattern))
        {
            fileNameFound = 1;
            break;
        }
    }
    if(fileNameFound)
            LocalParams.ImFileName.append(Line.substr(11));
    else
        return LocalParams;


    // read size of data vector
    bool namesLineFound = 0;
    while (inFile1.good())
    {
        getline(inFile1, Line);

        regex LinePattern("Tile Y.+");
        if (regex_match(Line.c_str(), LinePattern))
        {
            namesLineFound = 1;
            break;
        }
    }
    if(!namesLineFound)
        return LocalParams;

    LocalParams.ValueCount = 0;
    size_t stringPos = 0;
    while(1)
    {
        stringPos = Line.find("\t",stringPos);

        if(stringPos == string::npos)
            break;
        LocalParams.ValueCount++;
        stringPos++;
    }
    // read feature names
    std::stringstream InStringStream(Line);

    LocalParams.NamesVector.empty();
    char ColumnName[256];
    while(InStringStream.good())
    {
        InStringStream.getline(ColumnName,250,'\t');
        LocalParams.NamesVector.push_back(ColumnName);
    }

    //list<int> TilesParams;
    LocalParams.ParamsVect.empty();
//read directionalities
    while(inFile1.good())
    {
        TileParams params;
        getline(inFile1,Line);
        params.FromString(Line);
        if(params.tileX > -1 && params.tileY > -1)
            LocalParams.ParamsVect.push_back(params);
    }

    inFile1.close();
    return LocalParams;
}
//----------------------------------------------------------------------------------------------------------------
//FileParams MainWindow::GetStatisticalDataFromImage(path FileToOpen)

//----------------------------------------------------------------------------------------------------------------
cv::Mat MainWindow::ShowImage(cv::Mat Im, FileParams Params,
                           bool sudocolor,
                           bool showShape,
                           bool showLine,
                           float minIm,
                           float maxIm,
                           int tileLineThickness,
                           int featNr,
                           float meanIntensityTreshold,
                           double lineLength,
                           int imposedLineThickness)//,
                           //std::string ShowWindowName)

{
    Mat ImOut;
    ImOut = Mat::zeros(10,10,CV_8U);

    int maxX = Im.cols;
    int maxY = Im.rows;

    if(Im.empty())
        return ImOut;

    if(!maxX || ! maxY)
    {
        QMessageBox msgBox;
        ui->textEdit->append("\n Image File not exists");
        return ImOut;
    }
    Mat ImShow;
    Im.convertTo(Im,CV_16U);

    if(sudocolor)
        ImShow = ShowImage16PseudoColor(Im,minIm,maxIm);
    else
        ImShow = ShowImage16Gray(Im,minIm,maxIm);

    //

    int numOfDirections = (int)Params.ParamsVect.size();

    for(int i = 0; i < numOfDirections; i++)
    {
        int x  = Params.ParamsVect[i].tileX;
        int y  = Params.ParamsVect[i].tileY;
        double angle = Params.ParamsVect[i].Params[featNr];

        int lineOffsetX = (int)round(lineLength * sin(angle * PI / 180.0));
        int lineOffsetY = (int)round(lineLength * cos(angle * PI / 180.0));

        if (angle >= -600 && Params.ParamsVect[i].Params[3] >= meanIntensityTreshold )
        {
            if(showShape)
            {
                if(sudocolor)
                    ShowShape(ImShow, x, y, Params.tileShape, Params.tileSize, tileLineThickness,Scalar(0.0, 0.0, 0.0, 0.0));
                else
                    ShowShape(ImShow, x, y, Params.tileShape, Params.tileSize, tileLineThickness,Scalar(0.0, 0.0, 255.0, 0.0));
            }
            if(showLine)
            {
                if(sudocolor)
                    line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0.0, 0.0, 0.0, 0.0), imposedLineThickness);
                else
                    line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0.0, 0.0, 255.0, 0.0), imposedLineThickness);
            }
        }
    }
    return ImShow;
    //imshow(ShowWindowName.c_str(),ImShow);
}
//----------------------------------------------------------------------------------------------------------------
void MainWindow::ImageAnalysis(cv::Mat Im, FileParams *Params, unsigned short intensityThreshold)
{
    if(Im.empty())
        return;
    Im.convertTo(Im,CV_16U);
    Mat Roi;
    int roiMaxX, roiMaxY; // Bounding box sizes for ROI
    switch (Params->tileShape) // Different tile shapes
    {
    case 1: // Rectangle
        roiMaxX = Params->tileSize;
        roiMaxY = Params->tileSize;
        Roi = Mat::ones(roiMaxY, roiMaxX, CV_16U);
        break;
    case 2: // Ellipse
        roiMaxX = Params->tileSize;
        roiMaxY = Params->tileSize;
        Roi = Mat::zeros(roiMaxY, roiMaxX, CV_16U);
        ellipse(Roi, Point(roiMaxX / 2, roiMaxY / 2),
            Size(roiMaxX / 2, roiMaxY / 2), 0.0, 0.0, 360.0,
            1, -1);
        break;
    case 3: // Hexagon
    {
        int edgeLength = Params->tileSize / 2;
        roiMaxX = edgeLength * 2;
        roiMaxY = (int)((float)edgeLength * 0.8660254 * 2.0);
        Roi = Mat::zeros(roiMaxY, roiMaxX, CV_16U);

        Point vertice0(edgeLength / 2, 0);
        Point vertice1(edgeLength / 2 + edgeLength - 1, 0);
        Point vertice2(roiMaxX - 1, roiMaxY / 2);
        Point vertice3(edgeLength / 2 + edgeLength - 1, roiMaxY - 1);
        Point vertice4(edgeLength / 2, roiMaxY - 1);
        Point vertice5(0, roiMaxY / 2);

        line(Roi, vertice0, vertice1, 1, 1);
        line(Roi, vertice1, vertice2, 1, 1);
        line(Roi, vertice2, vertice3, 1, 1);
        line(Roi, vertice3, vertice4, 1, 1);
        line(Roi, vertice4, vertice5, 1, 1);
        line(Roi, vertice5, vertice0, 1, 1);
        unsigned short *wRoi;

        for (int y = 1; y < roiMaxY - 1; y++)
        {
            wRoi = (unsigned short *)Roi.data + roiMaxX * y;
            int x = 0;
            for (x; x < roiMaxX; x++)
            {
                if (*wRoi)
                    break;
                wRoi++;
            }
            x++;
            wRoi++;
            for (x; x < roiMaxX; x++)
            {
                if (*wRoi)
                    break;
                *wRoi = 1;
                wRoi++;
            }
        }

    }
    break;
    default:
        break;
    }
    Mat SmallIm;
    int numOfDirections = (int)Params->ParamsVect.size();
    for(int i = 0; i < numOfDirections; i++)
    {
        int x  = Params->ParamsVect[i].tileX;
        int y  = Params->ParamsVect[i].tileY;

        Im(Rect(x - roiMaxX/2, y - roiMaxY/2, roiMaxX , roiMaxY)).copyTo(SmallIm);
        unsigned short *wSmallIm =  (unsigned short *)SmallIm.data;
        unsigned short *wRoi =  (unsigned short *)Roi.data;
        int pixCount = 0;
        int sigCount = 0;
        for(int y = 0; y < roiMaxY; y++)
        {
            for(int x = 0; x < roiMaxX; x++)
            {
                if(*wRoi)
                {
                    pixCount++;
                    if(*wSmallIm > intensityThreshold)
                    {
                        sigCount++;
                    }

                }
                wRoi++;
                wSmallIm++;
            }

        }
        float percentage = float(sigCount)/float(pixCount)*100.0;
        Params->ParamsVect[i].Params[3] = percentage;
    }
}

//----------------------------------------------------------------------------------------------------------------
cv::Mat MainWindow::Show2Image(cv::Mat Im, cv::Mat Im2, FileParams Params, FileParams Params2,
                           bool sudocolor,
                           bool showShape,
                           bool showLine,
                           float minIm, float maxIm, float minIm2, float maxIm2,
                           int tileLineThickness,
                           int featNr,
                           float meanIntensityTreshold, float meanIntensityTreshold2,
                           double lineLength,
                           int imposedLineThickness)


{

    if(Im.empty())
        return Mat::zeros(10,20,CV_8U);
    if(Im2.empty())
        return Mat::zeros(10,20,CV_8U);
    int maxX = Im.cols;
    int maxY = Im.rows;

    if(!maxX || ! maxY)
    {
        QMessageBox msgBox;
        ui->textEdit->append("\n Image File not exists");
        return Mat::zeros(10,20,CV_8U);
    }
    Mat ImShow;
    int maxX2 = Im2.cols;
    int maxY2 = Im2.rows;

    if(!maxX2 || ! maxY2)
    {
        QMessageBox msgBox;
        ui->textEdit->append("\n Image File not exists");
        return Mat::zeros(10,20,CV_8U);
    }
    Mat ImShow2;
    //Im.convertTo(Im,CV_16U);
    //Im2.convertTo(Im2,CV_16U);
    if(sudocolor)
    {
        ImShow = ShowImage16PseudoColor(Im,minIm,maxIm);
        ImShow2 = ShowImage16PseudoColor(Im2,minIm2,maxIm2);
    }
    else
    {
        ImShow = ShowImage16Gray(Im,minIm,maxIm);
        ImShow2 = ShowImage16Gray(Im2,minIm2,maxIm2);
    }


    unsigned long long numOfDirections = Params.ParamsVect.size();

    for(unsigned long long i = 0; i < numOfDirections; i++)
    {
        int x  = Params.ParamsVect[i].tileX;
        int y  = Params.ParamsVect[i].tileY;
        double angle = Params.ParamsVect[i].Params[featNr];

        if(showShape)
        {
            if((Params.ParamsVect[i].Params[3] >= meanIntensityTreshold) && !(Params2.ParamsVect[i].Params[3]>= meanIntensityTreshold2))
            {
                if(sudocolor)
                {
                    ShowShape(ImShow, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(0.0, 0.0, 0.0, 0.0));
                    ShowShape(ImShow2, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(0.0, 0.0, 0.0, 0.0));
                }
                else
                {
                    ShowShape(ImShow, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(0.0, 250.0, 0.0, 0.0));
                    ShowShape(ImShow2, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(0.0, 250.0, 0.0, 0.0));
                }
            }
            if((Params.ParamsVect[i].Params[3] < meanIntensityTreshold) && (Params2.ParamsVect[i].Params[3]>= meanIntensityTreshold2))
            {
                if(sudocolor)
                {
                    ShowShape(ImShow, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(255.0, 255.0, 255.0, 0.0));
                    ShowShape(ImShow2, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(255.0, 255.0, 255.0, 0.0));
                }
                else
                {
                    ShowShape(ImShow, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(0, 0.0, 250.0, 0.0));
                    ShowShape(ImShow2, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(0, 0.0, 250.0, 0.0));
                }
            }
            if(Params.ParamsVect[i].Params[3] >= meanIntensityTreshold && (Params2.ParamsVect[i].Params[3]>= meanIntensityTreshold2))
            {
                if(sudocolor)
                {
                    ShowShape(ImShow, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(200, 200.0, 200.0, 0.0));
                    ShowShape(ImShow2, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(200, 200.0, 200.0, 0.0));
                }
                else
                {
                    ShowShape(ImShow, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(250, 100.0, 100.0, 0.0));
                    ShowShape(ImShow2, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(250, 100.0, 100.0, 0.0));
                }
            }

        }

        int lineOffsetX = (int)round(lineLength * sin(angle * PI / 180.0));
        int lineOffsetY = (int)round(lineLength * cos(angle * PI / 180.0));

        if (angle >= -600 && showLine && Params.ParamsVect[i].Params[3] >= meanIntensityTreshold )
        {
            if(sudocolor)
            {
                line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0, 0.0, 0.0, 0.0), imposedLineThickness);
                line(ImShow2, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0, 0.0, 0.0, 0.0), imposedLineThickness);
            }
            else
            {
                line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0, 250.0, 0.0, 0.0), imposedLineThickness);
                line(ImShow2, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0, 250.0, 0.0, 0.0), imposedLineThickness);
            }
        }
    }
    numOfDirections = (int)Params2.ParamsVect.size();

    for(int i = 0; i < numOfDirections; i++)
    {
        int x  = Params2.ParamsVect[i].tileX;
        int y  = Params2.ParamsVect[i].tileY;
        double angle = Params2.ParamsVect[i].Params[featNr];



        int lineOffsetX = (int)round(lineLength * sin(angle * PI / 180.0));
        int lineOffsetY = (int)round(lineLength * cos(angle * PI / 180.0));

        if (angle >= -600 && showLine && Params2.ParamsVect[i].Params[3] >= meanIntensityTreshold2 )
        {
            if(sudocolor)
            {
                line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(250.0, 0.0, 0.0, 0.0), imposedLineThickness);
                line(ImShow2, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(250.0, 0.0, 0.0, 0.0), imposedLineThickness);
            }
            else
            {
                line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0.0, 0.0, 250.0, 0.0), imposedLineThickness);
                line(ImShow2, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0.0, 0.0, 250.0, 0.0), imposedLineThickness);
            }

        }
    }

    //imshow("Input image 1",ImShow);
    //imshow("Input image 2",ImShow2);

    Mat ImOut = Mat::zeros(Size(maxX + maxX2,maxY) ,ImShow.type());
    ImShow.copyTo(ImOut(Rect(0, 0, ImShow.cols , ImShow.rows)));
    ImShow2.copyTo(ImOut(Rect(maxX, 0, ImShow2.cols , ImShow2.rows)));

    return ImOut;
    //imshow("Two images",ImOut);
}
//------------------------------------------------------------------------------------------------------------------------------
Mat SchowImageCobination(cv::Mat Im1, cv::Mat Im2, cv::Mat Im3, FileParams Params, FileParams Params2,
                         float minIm1, float maxIm1,
                         float minIm2, float maxIm2,
                         float minIm3, float maxIm3,
                         bool showIm1, bool showIm2, bool showIm3, bool showShape, bool showLine,
                         int tileLineThickness, int lineLength, int imposedLineThickness,
                         float meanIntensityTreshold, float meanIntensityTreshold2)

{
    if(Im1.empty())
        return Mat::zeros(1,1,CV_8UC3);

    int maxX = Im1.cols;
    int maxY = Im1.rows;
    int maxXY = maxX*maxY;

    if(Im2.empty())
        Im2 = Mat::zeros(maxY, maxX, CV_16U);
    if(Im3.empty())
        Im3 = Mat::zeros(maxY, maxX, CV_16U);;

    Mat ImShow = Mat::zeros(maxY, maxX, CV_8UC3);

    float difference1 = maxIm1 - minIm1;
    if(difference1 == 0)
        difference1 = 1;
    float gain1 = 255/difference1;
    float offset1 = gain1 * minIm1;

    float difference2 = maxIm2 - minIm2;
    if(difference2 == 0)
        difference2 = 1;
    float gain2 = 255/difference2;
    float offset2 = gain2 * minIm2;

    float difference3 = maxIm3 - minIm3;
    if(difference3 == 0)
        difference3 = 1;
    float gain3 = 255/difference3;
    float offset3 = gain3 * minIm3;


    float value;
    unsigned char index;


    unsigned short *wIm1 = (unsigned short *)Im1.data;
    unsigned short *wIm2 = (unsigned short *)Im2.data;
    unsigned short *wIm3 = (unsigned short *)Im3.data;
    unsigned char *wImShow = (unsigned char *)ImShow.data;

    for (int i = 0; i < maxXY; i++)
    {

        value = (float)(*wIm3) * gain3 - offset3;
        if (value > 255)
            value = 255;
        if (value < 0)
            value = 0;
        index = (char)floor(value);
        if(showIm3)
            *wImShow = index;
        else
            *wImShow = 0;
        wImShow++;

        value = (float)(*wIm2) * gain2 - offset2;
        if (value > 255)
            value = 255;
        if (value < 0)
            value = 0;
        index = (char)floor(value);
        if(showIm2)
            *wImShow = index;
        else
            *wImShow = 0;
        wImShow++;

        value = (float)(*wIm1) * gain1 - offset1;
        if (value > 255)
            value = 255;
        if (value < 0)
            value = 0;
        index = (char)floor(value);
        if(showIm1)
            *wImShow = index;
        else
            *wImShow = 0;
        wImShow++;
        wIm1++;
        wIm2++;
        wIm3++;
    }
    int numOfDirections = (int)Params.ParamsVect.size();

    for(int i = 0; i < numOfDirections; i++)
    {
        int x  = Params.ParamsVect[i].tileX;
        int y  = Params.ParamsVect[i].tileY;
        double angle = Params.ParamsVect[i].Params[0];

        if(showShape && Params.ParamsVect[i].Params[3] >= meanIntensityTreshold)
            ShowShape(ImShow, x, y, Params.tileShape, Params.tileSize, tileLineThickness, Scalar(255.0, 255.0, 255.0, 0.0));

        int lineOffsetX = (int)round(lineLength * sin(angle * PI / 180.0));
        int lineOffsetY = (int)round(lineLength * cos(angle * PI / 180.0));

        if (angle >= -600 && showLine && Params.ParamsVect[i].Params[3] >= meanIntensityTreshold )
        {
            line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(255.0, 255.0, 255.0, 0.0), imposedLineThickness);
        }
    }
    numOfDirections = (int)Params2.ParamsVect.size();

    for(int i = 0; i < numOfDirections; i++)
    {
        int x  = Params2.ParamsVect[i].tileX;
        int y  = Params2.ParamsVect[i].tileY;
        double angle = Params2.ParamsVect[i].Params[0];

        if(showShape && Params2.ParamsVect[i].Params[3] >= meanIntensityTreshold2)
            ShowShape(ImShow, x, y, Params2.tileShape, Params2.tileSize, tileLineThickness, Scalar(200.0, 200.0, 200.0, 0.0));

        int lineOffsetX = (int)round(lineLength * sin(angle * PI / 180.0));
        int lineOffsetY = (int)round(lineLength * cos(angle * PI / 180.0));

        if (angle >= -600 && showLine && Params2.ParamsVect[i].Params[3] >= meanIntensityTreshold2 )
        {
            line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(200.0, 200.0, 200.0, 0.0), imposedLineThickness);
        }
    }
    return ImShow;

}

//------------------------------------------------------------------------------------------------------------------------------
Mat SchowImageCobination(cv::Mat Im1, cv::Mat Im2, cv::Mat Im3,
                         float minIm1, float maxIm1,
                         float minIm2, float maxIm2,
                         float minIm3, float maxIm3,
                         bool showIm1, bool showIm2, bool showIm3)
//                          int tileLineThickness,
//                          float meanIntensityTreshold, float meanIntensityTreshold2)

{
    if(Im1.empty())
        return Mat::zeros(1,1,CV_8UC3);

    int maxX = Im1.cols;
    int maxY = Im1.rows;
    int maxXY = maxX*maxY;

    if(Im2.empty())
        Im2 = Mat::zeros(maxY, maxX, CV_16U);
    if(Im3.empty())
        Im3 = Mat::zeros(maxY, maxX, CV_16U);;

    Mat ImShow = Mat::zeros(maxY, maxX, CV_8UC3);

    float difference1 = maxIm1 - minIm1;
    if(difference1 == 0)
        difference1 = 1;
    float gain1 = 255/difference1;
    float offset1 = gain1 * minIm1;

    float difference2 = maxIm2 - minIm2;
    if(difference2 == 0)
        difference2 = 1;
    float gain2 = 255/difference2;
    float offset2 = gain2 * minIm2;

    float difference3 = maxIm3 - minIm3;
    if(difference3 == 0)
        difference3 = 1;
    float gain3 = 255/difference3;
    float offset3 = gain3 * minIm3;


    float value;
    unsigned char index;


    unsigned short *wIm1 = (unsigned short *)Im1.data;
    unsigned short *wIm2 = (unsigned short *)Im2.data;
    unsigned short *wIm3 = (unsigned short *)Im3.data;
    unsigned char *wImShow = (unsigned char *)ImShow.data;

    for (int i = 0; i < maxXY; i++)
    {

        value = (float)(*wIm3) * gain3 - offset3;
        if (value > 255)
            value = 255;
        if (value < 0)
            value = 0;
        index = (char)floor(value);
        if(showIm3)
            *wImShow = index;
        else
            *wImShow = 0;
        wImShow++;

        value = (float)(*wIm2) * gain2 - offset2;
        if (value > 255)
            value = 255;
        if (value < 0)
            value = 0;
        index = (char)floor(value);
        if(showIm2)
            *wImShow = index;
        else
            *wImShow = 0;
        wImShow++;

        value = (float)(*wIm1) * gain1 - offset1;
        if (value > 255)
            value = 255;
        if (value < 0)
            value = 0;
        index = (char)floor(value);
        if(showIm1)
            *wImShow = index;
        else
            *wImShow = 0;
        wImShow++;
        wIm1++;
        wIm2++;
        wIm3++;
    }
    return ImShow;

}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowFromVector(int vectPos, int offset, bool showIm1, bool showIm2, bool showIm3,bool offsetSecond)
{
    int maxVectPos1 = ImVect1.size() - 1;
    int maxVectPos2 = ImVect2.size() - 1;
    int maxVectPos3 = ImVect3.size() - 1;

    int localMaxX,localMaxY;
    Mat ImTemp1, ImTemp2, ImTemp3;
    FileParams ParamsTemp1, ParamsTemp2;

    int vectPos1;
    int vectPos2;
    int vectPos3;

    if(offsetSecond)
    {
        vectPos1 = vectPos + offset;
        vectPos2 = vectPos;
    }
    else
    {
        vectPos1 = vectPos;
        vectPos2 = vectPos + offset;

    }
    vectPos3 = vectPos1;

    if(vectPos1 <= maxVectPos1 && maxVectPos1 >= 0)
    {
        ImTemp1 = ImVect1[vectPos1];
        ParamsTemp1 = FileParVect1[vectPos1];
        localMaxX = ImTemp1.cols;
        localMaxX = ImTemp1.rows;
    }
    else
        return;


    if(vectPos2 <= maxVectPos2 && maxVectPos2 >= 0)
    {
        ImTemp2 = ImVect2[vectPos2];
        ParamsTemp2 = FileParVect2[vectPos2];
    }
    else
        ImTemp2.release();

    if(vectPos3 <= maxVectPos3 && maxVectPos2 >= 0)
    {
        ImTemp3 = ImVect3[vectPos3];
    }
    else
        ImTemp3.release();


    ImIn = ImTemp1;
    ImIn2 = ImTemp2;
    ImIn3 = ImTemp3;
    FilePar1 = ParamsTemp1;
    FilePar2 = ParamsTemp2;

    Mat ImTemp;
    medianBlur(ImIn,ImTemp,3);
    ImageAnalysis(ImTemp, &FilePar1, intensityThresholdIm1);

    medianBlur(ImIn2,ImTemp,3);
    ImageAnalysis(ImTemp, &FilePar2, intensityThresholdIm2);
/*
    imshow("From Vector",SchowImageCobination(ImIn, ImIn2, ImIn3, FilePar1, FilePar2,
                                              minIm, maxIm, minIm2, maxIm2, minIm3, maxIm3,
                                              showVectIm1,showVectIm2,showVectIm3, showShape, showLine,
                                              tileLineThickness, lineLength, imposedLineThickness,
                                              meanIntensityTreshold, meanIntensityTreshold2));
*/
    ShowImages();
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowXZFromVector(int yPosition)
{
    int maxZ = ImVect1.size();
    if(maxZ < 1)
        return;
    if (ImVect1[0].empty())
        return;
    int maxX = ImVect1[0].cols;
    int maxY = ImVect1[0].rows;
    if(maxX < 1)
        return;
    if(yPosition >= maxY)
        return;



    float difference1 = maxIm - minIm;
    if(difference1 == 0)
        difference1 = 1;
    float gain1 = 255/difference1;
    float offset1 = gain1 * minIm;

    float difference2 = maxIm2 - minIm2;
    if(difference2 == 0)
        difference2 = 1;
    float gain2 = 255/difference2;
    float offset2 = gain2 * minIm2;

    float difference3 = maxIm3 - minIm3;
    if(difference3 == 0)
        difference3 = 1;
    float gain3 = 255/difference3;
    float offset3 = gain3 * minIm3;

    bool vect2OK = true;
    if(ImVect2.size() != ImVect1.size())
        vect2OK = false;

    bool vect3OK = true;
    if(ImVect3.size() != ImVect1.size())
        vect3OK = false;


    unsigned short *wIm1,*wIm2,*wIm3;

    Mat ImShow = Mat::zeros(maxZ, maxX, CV_8UC3);
    unsigned char * wImShow = (unsigned char *)ImShow.data;
    float value;
    int index;
    for(int z = 0 ; z < maxZ; z++)
    {
        wIm1 = (unsigned short *)ImVect1[z].data + yPosition * maxX;

        if(vect2OK)
            wIm2 = (unsigned short *)ImVect2[z].data + yPosition * maxX;
        if(vect3OK)
            wIm3 = (unsigned short *)ImVect3[z].data + yPosition * maxX;

        for(int x = 0 ; x < maxX; x++)
        {
            if(vect3OK)
            {
                value = (float)(*wIm3) * gain3 - offset3;
                if (value > 255)
                    value = 255;
                if (value < 0)
                    value = 0;
                index = (char)floor(value);
                *wImShow = index;
            }
            else
                *wImShow = 0;
            wImShow++;

            if(vect2OK)
            {
                value = (float)(*wIm2) * gain2 - offset2;
                if (value > 255)
                    value = 255;
                if (value < 0)
                    value = 0;
                index = (char)floor(value);
                *wImShow = index;
            }
            else
                *wImShow = 0;
            wImShow++;

            value = (float)(*wIm1) * gain1 - offset1;
            if (value > 255)
                value = 255;
            if (value < 0)
                value = 0;
            index = (char)floor(value);
            *wImShow = index;
            wImShow++;

            wIm1++;
            wIm2++;
            wIm3++;
        }
    }

    imshow("Z view",ImShow);
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::FreeImageVectors()
{
    while(ImVect1.size() > 0)
    {
        ImVect1.back().release();

        ImVect1.pop_back();
    }
    while(ImVect2.size() > 0)
    {
        ImVect2.back().release();

        ImVect2.pop_back();
    }
    while(ImVect3.size() > 0)
    {
        ImVect3.back().release();

        ImVect3.pop_back();
    }
    while(FileParVect1.size() > 0)
    {
        FileParVect1.back().ParamsVect.clear();

        FileParVect1.pop_back();
    }
    while(FileParVect2.size() > 0)
    {
        FileParVect2.back().ParamsVect.clear();

        FileParVect2.pop_back();
    }

}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowImages()
{
    Mat ImToShow;
    if(showFirstImage)
    {
        ImToShow = ShowImage(ImIn, FilePar1, sudocolor, showShape, showLine, minIm, maxIm, tileLineThickness, featNr,
                               meanIntensityTreshold, lineLength, imposedLineThickness);
        if (imageScale !=1.0)
            cv::resize(ImToShow,ImToShow,Size(),imageScale,imageScale,INTER_NEAREST);
        imshow("In File 1",ImToShow);
    }
    if(showSecondImage)
    {
        ImToShow = ShowImage(ImIn2, FilePar2, sudocolor, showShape, showLine, minIm2, maxIm2, tileLineThickness, featNr,
                          meanIntensityTreshold2, lineLength, imposedLineThickness);
        if (imageScale !=1.0)
            cv::resize(ImToShow,ImToShow,Size(),imageScale,imageScale,INTER_NEAREST);
        imshow("In File 2",ImToShow);
    }
    if(showTwoImages)
    {
        ImToShow = Show2Image(ImIn, ImIn2, FilePar1, FilePar2, sudocolor, showShape, showLine, minIm, maxIm, minIm2, maxIm2,
                           tileLineThickness, featNr, meanIntensityTreshold, meanIntensityTreshold2, lineLength, imposedLineThickness);
        if (imageScale !=1.0)
            cv::resize(ImToShow,ImToShow,Size(),imageScale,imageScale,INTER_NEAREST);
        imshow("Two images",ImToShow);
    }
    if(showImageCombination)
    {
        ImToShow = SchowImageCobination(ImIn, ImIn2, ImIn3, FilePar1, FilePar2,
                                                          minIm, maxIm, minIm2, maxIm2, minIm3, maxIm3,
                                                          showVectIm1,showVectIm2,showVectIm3, showShape, showLine,
                                                          tileLineThickness, lineLength, imposedLineThickness,
                                                          meanIntensityTreshold, meanIntensityTreshold2);
        if (imageScale !=1.0)
            cv::resize(ImToShow,ImToShow,Size(),imageScale,imageScale,INTER_NEAREST);
        imshow("Combination",ImToShow);
    }
    //    imshow("Combination",SchowImageCobination(ImIn, ImIn2, ImIn3, minIm, maxIm, minIm2, maxIm2, minIm3, maxIm3, showVectIm1, showVectIm2, showVectIm3));

    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //        ui->checkBoxOffsetSecondIm->checkState());
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::OpenDirection1Directory()
{
    if (!exists(InputDirectory))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectory.string()+ " not exists ").c_str());
        msgBox.exec();
        InputDirectory = "d:\\";
    }
    if (!is_directory(InputDirectory))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectory.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        InputDirectory = "C:\\Data\\";
    }
    ui->DirectoryLineEdit->setText(QString::fromWCharArray(InputDirectory.wstring().c_str()));
    ui->FileListWidget->clear();
    for (directory_entry& FileToProcess : directory_iterator(InputDirectory))
    {
        regex FilePattern(ui->RegexLineEdit->text().toStdString());
        if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
            continue;

        path PathLocal = FileToProcess.path();
        if (!exists(PathLocal))
        {
            QMessageBox msgBox;
            msgBox.setText((PathLocal.filename().string() + " File not exists" ).c_str());
            msgBox.exec();
            break;
        }
        ui->FileListWidget->addItem(PathLocal.filename().string().c_str());
    }
        zFrame = 0;
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::OpenDirection2Directory()
{
    if (!exists(InputDirectory2))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectory2.string()+ " not exists ").c_str());
        msgBox.exec();
        InputDirectory2 = "d:\\";
    }
    if (!is_directory(InputDirectory))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectory2.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        InputDirectory2 = "C:\\Data\\";
    }
    ui->Directory2LineEdit->setText(QString::fromWCharArray(InputDirectory2.wstring().c_str()));
    ui->File2ListWidget->clear();
    for (directory_entry& FileToProcess : directory_iterator(InputDirectory2))
    {
        regex FilePattern(ui->RegexLineEdit->text().toStdString());
        if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
            continue;

        path PathLocal = FileToProcess.path();
        if (!exists(PathLocal))
        {
            QMessageBox msgBox;
            msgBox.setText((PathLocal.filename().string() + " File not exists" ).c_str());
            msgBox.exec();
            break;
        }
        ui->File2ListWidget->addItem(PathLocal.filename().string().c_str());
    }
    zFrame = 0;
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::OpenImage1Directory()
{
    if (!exists(InputDirectoryIm1))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm1.string()+ " not exists ").c_str());
        msgBox.exec();
        InputDirectoryIm1 = "d:\\";
    }
    if (!is_directory(InputDirectoryIm1))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm1.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        InputDirectoryIm1 = "C:\\Data\\";
    }
    ui->DirectoryIm1LineEdit->setText(QString::fromWCharArray(InputDirectoryIm1.wstring().c_str()));
    ui->FileIm1ListWidget->clear();
    for (directory_entry& FileToProcess : directory_iterator(InputDirectoryIm1))
    {
        regex FilePattern(ui->RegexImLineEdit->text().toStdString());
        if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
            continue;

        path PathLocal = FileToProcess.path();
        if (!exists(PathLocal))
        {
            QMessageBox msgBox;
            msgBox.setText((PathLocal.filename().string() + " File not exists" ).c_str());
            msgBox.exec();
            break;
        }
        ui->FileIm1ListWidget->addItem(PathLocal.filename().string().c_str());
    }
        zFrame = 0;
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::OpenImage2Directory()
{
    if (!exists(InputDirectoryIm2))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm2.string()+ " not exists ").c_str());
        msgBox.exec();
        InputDirectoryIm2 = "d:\\";
    }
    if (!is_directory(InputDirectoryIm2))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm2.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        InputDirectoryIm2 = "C:\\Data\\";
    }
    ui->DirectoryIm2LineEdit->setText(QString::fromWCharArray(InputDirectoryIm2.wstring().c_str()));
    ui->FileIm2ListWidget->clear();
    for (directory_entry& FileToProcess : directory_iterator(InputDirectoryIm2))
    {
        regex FilePattern(ui->RegexImLineEdit->text().toStdString());
        if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
            continue;

        path PathLocal = FileToProcess.path();
        if (!exists(PathLocal))
        {
            QMessageBox msgBox;
            msgBox.setText((PathLocal.filename().string() + " File not exists" ).c_str());
            msgBox.exec();
            break;
        }
        ui->FileIm2ListWidget->addItem(PathLocal.filename().string().c_str());
    }
    zFrame = 0;
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::OpenImage3Directory()
{
    if (!exists(InputDirectoryIm3))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm3.string()+ " not exists ").c_str());
        msgBox.exec();
        InputDirectoryIm3 = "d:\\";
    }
    if (!is_directory(InputDirectoryIm3))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm3.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        InputDirectoryIm3 = "C:\\Data\\";
    }
    ui->DirectoryIm3LineEdit->setText(QString::fromWCharArray(InputDirectoryIm3.wstring().c_str()));
    ui->FileIm3ListWidget->clear();
    for (directory_entry& FileToProcess : directory_iterator(InputDirectoryIm3))
    {
        regex FilePattern(ui->RegexImLineEdit->text().toStdString());
        if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
            continue;

        path PathLocal = FileToProcess.path();
        if (!exists(PathLocal))
        {
            QMessageBox msgBox;
            msgBox.setText((PathLocal.filename().string() + " File not exists" ).c_str());
            msgBox.exec();
            break;
        }
        ui->FileIm3ListWidget->addItem(PathLocal.filename().string().c_str());
    }
        zFrame = 0;

}

//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::on_pushButton_clicked()
{
    QFileDialog dialog(this, "Open Folder");
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory(InputDirectory.string().c_str());

    //QStringList FileList= dialog.e
    if(dialog.exec())
    {
        InputDirectory = dialog.directory().path().toStdWString();
    }
    else
        return;

    OpenDirection1Directory();
}

void MainWindow::on_FileListWidget_currentTextChanged(const QString &currentText)
{
    FileToOpen = InputDirectory;
    FileToOpen.append(currentText.toStdWString());
    FilePar1 = GetDirectionData(FileToOpen);

    ui->FileIm1ListWidget->setCurrentRow(ui->FileListWidget->currentRow());

    path imageToOpen = InputDirectoryIm1;//path imageToOpen = FilePar1.ImFolderName;
    imageToOpen.append(FilePar1.ImFileName.string());
    ImIn = imread(imageToOpen.string().c_str(),CV_LOAD_IMAGE_ANYDEPTH);
    if(ImIn.empty())
    {
        ui->textEdit->append("\nFailed To Open Image\n");
        ui->textEdit->append(imageToOpen.string().c_str());
        ui->textEdit->append("\n");
        return;
    }
    if(ImIn.type() != CV_16U)
    {
        ImIn.convertTo(ImIn,CV_16U);
    }
    medianBlur(ImIn,ImIn,3);
    ImageAnalysis(ImIn, &FilePar1, intensityThresholdIm1);
    ShowImages();
}

void MainWindow::on_checkBoxShowSape_toggled(bool checked)
{
    showShape = checked;
    ShowImages();
}

void MainWindow::on_checkBoxShowLine_toggled(bool checked)
{
    showLine = checked;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_checkBoxShowSudoColor_toggled(bool checked)
{
    sudocolor = checked;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_spinBoxImposedShapeThickness_valueChanged(int arg1)
{
    tileLineThickness = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_spinBoxImposedLineThickness_valueChanged(int arg1)
{
    imposedLineThickness = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_spinBoxLineLength_valueChanged(int arg1)
{
    lineLength = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_spinBoxFeatureToShow_valueChanged(int arg1)
{
    featNr = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_pushButtonChoseOutDir_clicked()
{
    QFileDialog dialog(this, "Open Folder");
    dialog.setFileMode(QFileDialog::Directory);
    //dialog.setDirectory("C:/Data");

    //QStringList FileList= dialog.e
    if(dialog.exec())
    {
        OutputDirectory = dialog.directory().path().toStdWString();
    }
    else
        return;
    if (!exists(OutputDirectory))
    {
        QMessageBox msgBox;
        msgBox.setText((OutputDirectory.string()+ " not exists ").c_str());
        msgBox.exec();
        InputDirectory = "C:\\";
    }
    if (!is_directory(OutputDirectory))
    {
        QMessageBox msgBox;
        msgBox.setText((OutputDirectory.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        OutputDirectory = "C:\\Data\\";
    }
    ui->LineEditSaveDirectory->setText(QString::fromWCharArray(OutputDirectory.wstring().c_str()));
}

void MainWindow::on_pushButtonSaveOut_pressed()
{
    if (!exists(FileToOpen))
        return;

    if(!ImShow.cols || !ImShow.rows)
    {
        return;
    }
    path OutFileName;

    //OutFileName = OutputDirectory;
    //OutFileName.append(ImFileName.stem().string() + ".bmp");
    //imwrite(OutFileName.string(),ImShow);



    if(showImageCombination)
    {
        OutFileName = OutputDirectory;
        OutFileName.append(ImFileName.stem().string() + "Combination.bmp");
        imwrite(OutFileName.string(),SchowImageCobination(ImIn, ImIn2, ImIn3, FilePar1, FilePar2,
                                                          minIm, maxIm, minIm2, maxIm2, minIm3, maxIm3,
                                                          showVectIm1,showVectIm2,showVectIm3, showShape, showLine,
                                                          tileLineThickness, lineLength, imposedLineThickness,
                                                          meanIntensityTreshold, meanIntensityTreshold2));
    }

}

void MainWindow::on_doubleSpinBoxImMin_valueChanged(double arg1)
{
    minIm = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_doubleSpinBoxImMax_valueChanged(double arg1)
{
    maxIm = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_doubleSpinBoxProcTresh_valueChanged(double arg1)
{
    meanIntensityTreshold = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_pushButton2_clicked()
{
    QFileDialog dialog(this, "Open Folder");
    dialog.setFileMode(QFileDialog::Directory);
    //dialog.setDirectory("C:/Data/Sumona3Out/D28/20150819_28d_SC1_A2_Calc_PermOCN_ActinDAPI_1/");

    //QStringList FileList= dialog.e
    if(dialog.exec())
    {
        InputDirectory2 = dialog.directory().path().toStdWString();
    }
    else
        return;
    OpenDirection2Directory();

}

void MainWindow::on_File2ListWidget_currentTextChanged(const QString &currentText)
{
    FileToOpen2 = InputDirectory2;
    FileToOpen2.append(currentText.toStdWString());
    FilePar2 = GetDirectionData(FileToOpen2);

    path imageToOpen = InputDirectoryIm2;//path imageToOpen = FilePar2.ImFolderName;

    imageToOpen.append(FilePar2.ImFileName.string());
    ImIn2 = imread(imageToOpen.string().c_str(),CV_LOAD_IMAGE_ANYDEPTH);
    if(ImIn.type() != CV_16U)
    {
        ImIn.convertTo(ImIn,CV_16U);
    }
    medianBlur(ImIn2,ImIn2,3);
    ImageAnalysis(ImIn2, &FilePar2, intensityThresholdIm2);
    ShowImages();
    //ProcessImage();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_doubleSpinBoxImMin2_valueChanged(double arg1)
{
    minIm2 = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_doubleSpinBoxImMax2_valueChanged(double arg1)
{
    maxIm2 = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_doubleSpinBoxProcTresh2_valueChanged(double arg1)
{
    meanIntensityTreshold2 = arg1;
    medianBlur(ImIn,ImIn,3);
    ImageAnalysis(ImIn, &FilePar1, intensityThresholdIm1);
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_checkBoxShowSecondIm_toggled(bool checked)
{
    showSecondImage = checked;
    if(showSecondImage)
        ShowImages();
        //ShowImage(ImIn2, FilePar2, sudocolor, showShape, showLine, minIm2, maxIm2, tileLineThickness, featNr,
        //          meanIntensityTreshold2, lineLength, imposedLineThickness, "In File 2");
    else
        destroyWindow("In File 2");
}


void MainWindow::on_checkBoxShowFirstIm_toggled(bool checked)
{
    showFirstImage = checked;
    if(showFirstImage)
        ShowImages();
        //ShowImage(ImIn, FilePar1, sudocolor, showShape, showLine, minIm, maxIm, tileLineThickness, featNr,
        //          meanIntensityTreshold, lineLength, imposedLineThickness, "In File 1");
    else
        destroyWindow("In File 1");
}

void MainWindow::on_checkBoxShowTwoIm_toggled(bool checked)
{
    showTwoImages = checked;
    if(showTwoImages)
        ShowImages();
        //Show2Image(ImIn, ImIn2, FilePar1, FilePar2, sudocolor, showShape, showLine, minIm, maxIm, minIm2, maxIm2,
        //           tileLineThickness, featNr, meanIntensityTreshold, meanIntensityTreshold2, lineLength, imposedLineThickness);
    else
        destroyWindow("Two images");
}

void MainWindow::on_pushButtonPlus_clicked()
{
    /*
    if((zFrame + zOffset) > )
        zFrame = ui->FileListWidget->count() - zOffset;
    //z = ui->FileListWidget->currentRow();
    */
    zFrame++;

    int rowCount = ui->FileListWidget->count();

    if( zFrame >= rowCount)
        zFrame = rowCount - 1;
    if((zFrame + zOffset)>= rowCount)
        zFrame = rowCount - zOffset -1;

    ui->FileListWidget->setCurrentRow(zFrame);
    ui->File2ListWidget->setCurrentRow(zFrame+zOffset);
}

void MainWindow::on_spinBoxZOffset_valueChanged(int arg1)
{
    zOffset = arg1;
    int rowCount = ui->FileListWidget->count();

    if( zFrame >= rowCount)
        zFrame = rowCount - 1;
    if((zFrame + zOffset)>= rowCount)
        zFrame = rowCount - zOffset -1;

    if( zFrame < 0)
        zFrame = 0;
    if((zFrame + zOffset)< 0)
        zFrame = 0 - zOffset;


    ui->FileListWidget->setCurrentRow(zFrame);
    ui->File2ListWidget->setCurrentRow(zFrame + zOffset);
}

void MainWindow::on_pushButtonMinus_clicked()
{
    zFrame--;

    if( zFrame < 0)
        zFrame = 0;
    if((zFrame + zOffset)< 0)
        zFrame = 0 - zOffset;

    ui->FileListWidget->setCurrentRow(zFrame);
    ui->File2ListWidget->setCurrentRow(zFrame+zOffset);

}

void MainWindow::on_pushButtonCreateOut_clicked()
{
    int zCount = ImVect1.size();
    if(!zCount)
    {
        ui->textEdit->append("Empty first image vector");
        return;
    }
    if(ImVect2.size() != zCount)
    {
        ui->textEdit->append("Improper size of second image vector");
        return;
    }
    if(FileParVect1.size() != zCount)
    {
        ui->textEdit->append("Improper size of first parameter vector");
        return;
    }
    if(FileParVect2.size() != zCount)
    {
        ui->textEdit->append("Improper size of second parameter vector");
        return;
    }

    for(int z = 0; z < zCount ;z++)
    {
        Mat LocalIm1;
        ImVect1[z].copyTo(LocalIm1);
        //FileParams Params1 = FileParVect1[z];
        medianBlur(LocalIm1,LocalIm1,3);
        ImageAnalysis(LocalIm1, &FileParVect1[z], intensityThresholdIm1);

        Mat LocalIm2;
        ImVect2[z].copyTo(LocalIm2);
        //FileParams Params2 = FileParVect2[z];
        medianBlur(LocalIm2,LocalIm2,3);
        ImageAnalysis(LocalIm2, &FileParVect2[z], intensityThresholdIm2);

        LocalIm1.release();
        LocalIm1.release();

    }


    int zOffsetMin = -10;
    int zOffsetMax = 25;
    int zOffsetCount = zOffsetMax - zOffsetMin + 1;

    int *zOffsetValue = new int[zOffsetCount];
    int *ActinTiles = new int[zOffsetCount];
    int *CalceinTiles = new int[zOffsetCount];
    int *CoexistingTiles = new int[zOffsetCount];

    double *MeanDirrDiff = new double[zOffsetCount];
    double *StdDirrDiff = new double[zOffsetCount];

    double *ActinMainDir = new double[zOffsetCount];
    double *ActinSpread = new double[zOffsetCount];
    double *CalceinMainDir = new double[zOffsetCount];
    double *CalceinSpread = new double[zOffsetCount];

    double *ActinWCalceinMainDir = new double[zOffsetCount];
    double *ActinWCalceinSpread = new double[zOffsetCount];
    double *ActinNCalceinMainDir = new double[zOffsetCount];
    double *ActinNCalceinSpread = new double[zOffsetCount];

    double *CalceinWActinMainDir = new double[zOffsetCount];
    double *CalceinWActinSpread = new double[zOffsetCount];
    double *CalceinNActinMainDir = new double[zOffsetCount];
    double *CalceinNActinSpread = new double[zOffsetCount];


    /*
        zOffsetValue
        ActinTiles
        CalceinTiles
        CoexistingTiles
     */

    for(int i = 0; i < zOffsetCount; i++)
    {
        zOffsetValue[i] = 0;
        ActinTiles[i] = 0;
        CalceinTiles[i] = 0;
        CoexistingTiles[i] = 0;
    }

    int *AcitinDirectionHistogramOverall  = new int[180];
    int *CalceinDirectionHistogramOverall = new int[180];

    int *AcitinDirectionHistogramCoexist  = new int[180];
    int *CalceinDirectionHistogramCoexist = new int[180];

    int *AcitinDirectionHistogramNCoexist  = new int[180];
    int *CalceinDirectionHistogramNCoexist = new int[180];

    int *DirectionDifferenceHistogram = new int[91];


    string StrOut1 = "Actin Dir Histograms \n";
    string StrOut2 = "Calcein Dir Histograms \n";
    string StrOut3 = "Actin w Calcein Dir Histograms \n";
    string StrOut4 = "Actin n Calcein Dir Histograms \n";
    string StrOut5 = "Calcein w Actin Dir Histograms \n";
    string StrOut6 = "Calcein n Actin Dir Histograms \n";
    string StrOut7 = "Direction Difference Histograms \n";

    for(int zOffsetIndex = 0;zOffsetIndex < zOffsetCount; zOffsetIndex++)
    {
        int zOffset = zOffsetIndex + zOffsetMin;
        zOffsetValue[zOffsetIndex] = zOffset;

        int zStart = 0;
        int zStop = zCount;

        if((zStart + zOffset)< 0)
            zStart = 0 - zOffset;

        if((zStop + zOffset)>= zCount)
            zStop = zCount - zOffset;

        for(int i = 0; i < 180; i++)
        {
            AcitinDirectionHistogramOverall[i] = 0;
            CalceinDirectionHistogramOverall[i] = 0;

            AcitinDirectionHistogramCoexist[i] = 0;
            CalceinDirectionHistogramCoexist[i] = 0;

            AcitinDirectionHistogramNCoexist[i] = 0;
            CalceinDirectionHistogramNCoexist[i] = 0;
        }



        for(int i = 0; i < 91; i++)
        {
            DirectionDifferenceHistogram[i] = 0;
        }

        for(int z = zStart; z < zStop ;z++)
        {
            FileParams Params1 = FileParVect1[z];
            FileParams Params2 = FileParVect2[z + zOffset];

            int numOfTiles = Params1.ParamsVect.size();
            if (!numOfTiles )
            {
                ui->textEdit->append("Improper number of tiles in first set");
                continue;
            }

            if (numOfTiles != Params2.ParamsVect.size())
            {
                ui->textEdit->append("Improper number of tiles in second set");
                continue;
            }
            for(int t = 0; t < numOfTiles; t++)
            {
                int x  = Params1.ParamsVect[t].tileX;
                int y  = Params1.ParamsVect[t].tileY;

                int angle1 = Params1.ParamsVect[t].Params[0];
                int angle2 = Params2.ParamsVect[t].Params[0];
                float meanInt1 = Params1.ParamsVect[t].Params[3];
                float meanInt2 = Params2.ParamsVect[t].Params[3];

                bool actinSignal = false;
                if(meanInt1 >= meanIntensityTreshold)
                {
                    ActinTiles[zOffsetIndex]++;
                    actinSignal = true;
                    AcitinDirectionHistogramOverall[angle1]++;
                }

                bool calceinSignal = false;
                if(meanInt2 >= meanIntensityTreshold2)
                {
                    CalceinTiles[zOffsetIndex]++;
                    calceinSignal = true;
                    CalceinDirectionHistogramOverall[angle2]++;
                }

                bool coexistingSignal = false;
                if(calceinSignal && actinSignal)
                {
                    CoexistingTiles[zOffsetIndex]++;
                    AcitinDirectionHistogramCoexist[angle1]++;
                    CalceinDirectionHistogramCoexist[angle2]++;

                    coexistingSignal = true;

                    int diff;
                    if(angle1 > angle2)
                        diff = angle1 - angle2;
                    else
                        diff = angle2 - angle1;

                    if (diff > 90)
                        diff = 180-diff;
                    DirectionDifferenceHistogram[diff]++;
                }

                if(!calceinSignal && actinSignal)
                {
                    AcitinDirectionHistogramNCoexist[angle1]++;
                }

                if(calceinSignal && !actinSignal)
                {
                    CalceinDirectionHistogramNCoexist[angle2]++;
                }
            }
            //LocalIm1.release();
            //LocalIm1.release();



        }

        MeanDirrDiff[zOffsetIndex] = MeanOfDirDiffHistogram(DirectionDifferenceHistogram);
        StdDirrDiff[zOffsetIndex] = StdOfDirDiffHistogram(DirectionDifferenceHistogram, MeanDirrDiff[zOffsetIndex]);


        ActinMainDir[zOffsetIndex] = FindResultingDirection(AcitinDirectionHistogramOverall);
        ActinSpread[zOffsetIndex] = FindSpread(AcitinDirectionHistogramOverall, ActinMainDir[zOffsetIndex]);
        CalceinMainDir[zOffsetIndex] = FindResultingDirection(CalceinDirectionHistogramOverall);
        CalceinSpread[zOffsetIndex] = FindSpread(CalceinDirectionHistogramOverall, CalceinMainDir[zOffsetIndex]);;

        ActinWCalceinMainDir[zOffsetIndex] = FindResultingDirection(AcitinDirectionHistogramCoexist);
        ActinWCalceinSpread[zOffsetIndex] = FindSpread(AcitinDirectionHistogramCoexist, ActinWCalceinMainDir[zOffsetIndex]);
        ActinNCalceinMainDir[zOffsetIndex] = FindResultingDirection(AcitinDirectionHistogramNCoexist);
        ActinNCalceinSpread[zOffsetIndex] = FindSpread(AcitinDirectionHistogramNCoexist, ActinNCalceinMainDir[zOffsetIndex]);

        CalceinWActinMainDir[zOffsetIndex] = FindResultingDirection(CalceinDirectionHistogramCoexist);
        CalceinWActinSpread[zOffsetIndex] = FindSpread(CalceinDirectionHistogramCoexist, CalceinWActinMainDir[zOffsetIndex]);
        CalceinNActinMainDir[zOffsetIndex] = FindResultingDirection(CalceinDirectionHistogramNCoexist);
        CalceinNActinSpread[zOffsetIndex] = FindSpread(CalceinDirectionHistogramNCoexist, CalceinNActinMainDir[zOffsetIndex]);


        StrOut1 += DirHistogramToString(AcitinDirectionHistogramOverall);
        StrOut1 += "\n";

        StrOut2 += DirHistogramToString(CalceinDirectionHistogramOverall);
        StrOut2 += "\n";

        StrOut3 += DirHistogramToString(AcitinDirectionHistogramCoexist);
        StrOut3 += "\n";

        StrOut4 += DirHistogramToString(AcitinDirectionHistogramNCoexist);
        StrOut4 += "\n";

        StrOut5 += DirHistogramToString(CalceinDirectionHistogramCoexist);
        StrOut5 += "\n";

        StrOut6 += DirHistogramToString(CalceinDirectionHistogramNCoexist);
        StrOut6 += "\n";

        StrOut7 += DirDifferenceHistogramToString(DirectionDifferenceHistogram);
        StrOut7 += "\n";
    }

    delete[] AcitinDirectionHistogramOverall;
    delete[] CalceinDirectionHistogramOverall;

    delete[] AcitinDirectionHistogramCoexist;
    delete[] CalceinDirectionHistogramCoexist;

    delete[] AcitinDirectionHistogramNCoexist;
    delete[] CalceinDirectionHistogramNCoexist;

    delete[] DirectionDifferenceHistogram;


    string StrOut;

    StrOut = "zOffeset\t#ActinSignalTiles\t#CalceinSignalTiles\t#CoexsistingSignalTiles\t";
    StrOut += "MeanDifff\tStdDifff\t";
    StrOut += "Actin Main Dir\tActin Spread\tCalcein Main Dir\t Calcein Spread\t";
    StrOut += "Actin w Calcein Main Dir\tActin w Calcein Spread\tCalcein w Actin Main Dir\t Calcein w Actin Spread\t";
    StrOut += "Actin n Calcein Main Dir\tActin n Calcein Spread\tCalcein n Actin Main Dir\t Calcein n Actin Spread\n";
    for(int i = 0; i < zOffsetCount; i++)
    {
        StrOut += to_string(zOffsetValue[i]) + "\t";
        StrOut += to_string(ActinTiles[i]) + "\t";
        StrOut += to_string(CalceinTiles[i]) + "\t";
        StrOut += to_string(CoexistingTiles[i]) + "\t";

        StrOut += OutputStringFromNumber(MeanDirrDiff[i]) + "\t";
        StrOut += OutputStringFromNumber(StdDirrDiff[i]) + "\t";

        StrOut += OutputStringFromNumber(ActinMainDir[i]) + "\t";
        StrOut += OutputStringFromNumber(ActinSpread[i]) + "\t";
        StrOut += OutputStringFromNumber(CalceinMainDir[i]) + "\t";
        StrOut += OutputStringFromNumber(CalceinSpread[i]) + "\t";

        StrOut += OutputStringFromNumber(ActinWCalceinMainDir[i]) + "\t";
        StrOut += OutputStringFromNumber(ActinWCalceinSpread[i]) + "\t";
        StrOut += OutputStringFromNumber(ActinNCalceinMainDir[i]) + "\t";
        StrOut += OutputStringFromNumber(ActinNCalceinSpread[i]) + "\t";

        StrOut += OutputStringFromNumber(CalceinWActinMainDir[i]) + "\t";
        StrOut += OutputStringFromNumber(CalceinWActinSpread[i]) + "\t";
        StrOut += OutputStringFromNumber(CalceinNActinMainDir[i]) + "\t";
        StrOut += OutputStringFromNumber(CalceinNActinSpread[i]) + "\n";


    }



    path OutFileName = OutputDirectory;
    OutFileName.append("Summary ofset.txt");
    std::ofstream out(OutFileName.string().c_str());
    //std::ofstream out("L:\\ZTest2.txt");
    out << StrOut;
    out.close();
    StrOut.empty();

    path OutFileName2 = OutputDirectory;
    OutFileName2.append("Histograms ofset.txt");
    std::ofstream out2(OutFileName2.string().c_str());
    //std::ofstream out("L:\\ZTest2.txt");
    out2 << StrOut1;
    out2 << StrOut2;
    out2 << StrOut3;
    out2 << StrOut4;
    out2 << StrOut5;
    out2 << StrOut6;
    out2 << StrOut7;
    out2.close();

    StrOut1.empty();
    StrOut2.empty();
    StrOut3.empty();
    StrOut4.empty();
    StrOut5.empty();
    StrOut6.empty();
    StrOut7.empty();

    delete[] MeanDirrDiff;
    delete[] StdDirrDiff;

    delete[] zOffsetValue;
    delete[] ActinTiles;
    delete[] CalceinTiles;
    delete[] CoexistingTiles;
    delete[] ActinMainDir;
    delete[] CalceinMainDir;
    delete[] CalceinSpread;

    delete[] ActinWCalceinMainDir;
    delete[] ActinWCalceinSpread;
    delete[] ActinNCalceinMainDir;
    delete[] ActinNCalceinSpread;

    delete[] CalceinWActinMainDir;
    delete[] CalceinWActinSpread;
    delete[] CalceinNActinMainDir;
    delete[] CalceinNActinSpread;
}



void MainWindow::on_pushCreateHist_clicked()
{
    int start;
    int stop;
    int rowCount = ui->FileListWidget->count();

    start = 0;

    stop = rowCount;

    float maxMean = 0.0;
    float minMean = 1000000.0;

    for(int k = start; k < stop ;k++)
    {
        path LocalFileToOpen;

        LocalFileToOpen  = InputDirectory;
        LocalFileToOpen.append(ui->FileListWidget->item(k)->text().toStdWString());
        FileParams Params = GetDirectionData(LocalFileToOpen);

        int numOfDirections = Params.ParamsVect.size();
        for(int i = 0; i < numOfDirections; i++)
        {
            float meanInt = Params.ParamsVect[i].Params[9];
            if(maxMean < meanInt)
                maxMean = meanInt;
            if(minMean > meanInt)
                minMean = meanInt;
        }
    }


    float maxHist = maxMean;
    if(maxHist < 255)
        maxHist = 255;
    maxHist += 1;
    int Hist[256];
    for(int k = 0; k < 256 ;k++)
        Hist[k] = 0;

    float coefHist = 256.0 / maxHist;
    for(int k = start; k < stop ;k++)
    {
        path LocalFileToOpen;

        LocalFileToOpen  = InputDirectory;
        LocalFileToOpen.append(ui->FileListWidget->item(k)->text().toStdWString());
        FileParams Params = GetDirectionData(LocalFileToOpen);

        int numOfDirections = Params.ParamsVect.size();
        for(int i = 0; i < numOfDirections; i++)
        {
            float meanInt = Params.ParamsVect[i].Params[9];
            int index  = floor(meanInt * coefHist);
            if (index > 255)
                index = 255;
            Hist[index]++;
        }
    }
    string StrOut = "";
    StrOut += "Min Mean\t" + to_string(minMean) + "\n";
    StrOut += "Max Mean\t" + to_string(maxMean) + "\n";
    StrOut += "Max Hist\t" + to_string(maxHist) + "\n";
    StrOut += "k\tIntensity\tHist\n";
    for(int k = 0; k < 256 ;k++)
    {
        StrOut += to_string(k) + "\t" ;
        StrOut += to_string((float)k / coefHist) + "\t" ;
        StrOut += to_string(Hist[k]) + "\n" ;

    }
    //ui->textEdit->append(StrOut.c_str());
    path OutFileName = OutputDirectory;
    OutFileName.append("HistActin.txt");
    std::ofstream out(OutFileName.string().c_str());
    out << StrOut;
    out.close();
    StrOut.empty();

}

void MainWindow::on_pushButtonCreateGlobalHist_clicked()
{
    int start;
    int stop;
    int rowCount = ui->FileListWidget->count();

    start = 0;

    stop = rowCount;

    unsigned short max = 0;
    unsigned short min = 65535;

    int *Hist = new int[2048];
    for(int k = 0; k < 2048 ;k++)
        Hist[k] = 0;



    for(int k = start; k < stop ;k++)
    {
        path LocalFileToOpen;

        LocalFileToOpen  = InputDirectory;
        LocalFileToOpen.append(ui->FileListWidget->item(k)->text().toStdWString());
        FileParams Params = GetDirectionData(LocalFileToOpen);

        //int numOfDirections = Params.ParamsVect.size();
        path ImFile = Params.ImFolderName;
        ImFile.append(Params.ImFileName.string());
        string FileName = ImFile.string();
        if(!exists(ImFile))
            continue;
        Mat ImLocal = imread(FileName,CV_LOAD_IMAGE_ANYDEPTH);
        if(ImIn.type() != CV_16U)
        {
            ImIn.convertTo(ImIn,CV_16U);
        }

        int maxX = ImLocal.cols;
        int maxY = ImLocal.rows;
        int maxXY = maxX*maxY;
        unsigned short *wImLocal = (unsigned short*)ImLocal.data;

        for(int i = 0; i < maxXY; i++)
        {
            unsigned short intensity = *wImLocal;
            int index = (int)intensity/32;
            if(max < intensity)
                max = intensity;
            if(min > intensity)
                min = intensity;
            Hist[index]++;
            wImLocal++;

        }
    }

    string StrOut = "";
    StrOut += "Min Mean\t" + to_string(min) + "\n";
    StrOut += "Max Mean\t" + to_string(max) + "\n";
    //StrOut += "Max Hist\t" + to_string(maxHist) + "\n";
    StrOut += "k\tIntensity\tHist\n";
    for(int k = 0; k < 2048 ;k++)
    {
        StrOut += to_string(k) + "\t" ;
        StrOut += to_string(k * 32) + "\t" ;
        StrOut += to_string(Hist[k]) + "\n" ;

    }
    //ui->textEdit->append(StrOut.c_str());
    path OutFileName = OutputDirectory;
    OutFileName.append("HistActin.txt");
    std::ofstream out(OutFileName.string().c_str());
    out << StrOut;
    out.close();
    StrOut.empty();


    delete Hist;

}

void MainWindow::on_pushButtonStackHist64k_clicked()
{
    int startA;
    int stopA;
    int rowCountA = ui->FileListWidget->count();

    startA = 0;

    stopA = rowCountA;

    unsigned short maxA = 0;
    unsigned short minA = 65535;
    int pixCountA = 0;
    int fileCountA = 0;

    int *HistA = new int[65536];
    for(int k = 0; k < 65536 ;k++)
        HistA[k] = 0;



    for(int k = startA; k < stopA ;k++)
    {

        path LocalFileToOpenA;
        LocalFileToOpenA  = InputDirectory;
        LocalFileToOpenA.append(ui->FileListWidget->item(k)->text().toStdWString());
        FileParams Params = GetDirectionData(LocalFileToOpenA);

        //int numOfDirections = Params.ParamsVect.size();
        path ImFile = Params.ImFolderName;
        ImFile.append(Params.ImFileName.string());
        string FileName = ImFile.string();

        if(!exists(ImFile))
            continue;
        Mat ImLocal = imread(FileName,CV_LOAD_IMAGE_ANYDEPTH);
        if(ImIn.type() != CV_16U)
        {
            ImIn.convertTo(ImIn,CV_16U);
        }
        //ImLocal.convertTo(ImLocal,CV_16U);
        if(ImLocal.empty())
            continue;

        fileCountA++;

        int maxX = ImLocal.cols;
        int maxY = ImLocal.rows;
        int maxXY = maxX*maxY;
        unsigned short *wImLocal = (unsigned short*)ImLocal.data;

        for(int i = 0; i < maxXY; i++)
        {
            pixCountA++;
            unsigned short intensity = *wImLocal;
            int index = (int)intensity;
            if(maxA < intensity)
                maxA = intensity;
            if(minA > intensity)
                minA = intensity;
            HistA[index]++;
            wImLocal++;

        }
    }

    int maxHistAPos = 0;
    int maxHistAVal = 0;

    for(int k = 0; k < 65536 ;k++)
    {
        if(maxHistAVal < HistA[k])
        {
            maxHistAVal = HistA[k];
            maxHistAPos = k;
        }
    }



    int rowCountC = ui->File2ListWidget->count();

    int startC = 0;

    int stopC = rowCountC;

    int maxC = 0;
    int minC = 65535;
    int pixCountC = 0;
    int fileCountC = 0;

    int *HistC = new int[65536];
    for(int k = 0; k < 65536 ;k++)
        HistC[k] = 0;



    for(int k = startC; k < stopC ;k++)
    {

        path LocalFileToOpenC;
        LocalFileToOpenC  = InputDirectory2;
        LocalFileToOpenC.append(ui->File2ListWidget->item(k)->text().toStdWString());
        FileParams Params = GetDirectionData(LocalFileToOpenC);

        //int numOfDirections = Params.ParamsVect.size();
        path ImFile = Params.ImFolderName;
        ImFile.append(Params.ImFileName.string());
        string FileName = ImFile.string();

        if(!exists(ImFile))
            continue;
        Mat ImLocal = imread(FileName,CV_LOAD_IMAGE_ANYDEPTH);
        if(ImIn.type() != CV_16U)
        {
            ImIn.convertTo(ImIn,CV_16U);
        }
        //ImLocal.convertTo(ImLocal,CV_16U);
        if(ImLocal.empty())
            continue;

        fileCountC++;

        int maxX = ImLocal.cols;
        int maxY = ImLocal.rows;
        int maxXY = maxX*maxY;
        unsigned short *wImLocal = (unsigned short*)ImLocal.data;

        for(int i = 0; i < maxXY; i++)
        {
            pixCountC++;
            unsigned short intensity = *wImLocal;
            int index = (int)intensity;
            if(maxC < intensity)
                maxC = intensity;
            if(minC > intensity)
                minC = intensity;
            HistC[index]++;
            wImLocal++;

        }
    }

    int maxHistCPos = 0;
    int maxHistCVal = 0;

    for(int k = 0; k < 65536 ;k++)
    {
        if(maxHistCVal < HistC[k])
        {
            maxHistCVal = HistC[k];
            maxHistCPos = k;
        }
    }

    string StrOut = " val name \tActin \tCalcein\n";
    StrOut += "FolderName\t" + InputDirectory.string() + "\t" + InputDirectory2.string() + "\n";
    StrOut += "Pixel Count\t" + to_string(pixCountA)  + "\t" + to_string(pixCountC) + "\n";
    StrOut += "File Count\t"  + to_string(fileCountA) + "\t" + to_string(fileCountC) + "\n";
    StrOut += "Max Hist Position\t" + to_string(maxHistAPos) + "\t" + to_string(maxHistCPos) + "\n";
    StrOut += "Max Hist Val\t" + to_string(maxHistAVal) + "\t" + to_string(maxHistCVal) + "\n";

    StrOut += "Min Mean\t" + to_string(minA) + "\t" + to_string(minC) + "\n";
    StrOut += "Max Mean\t" + to_string(maxA) + "\t" + to_string(maxC) + "\n";
    //StrOut += "Max Hist\t" + to_string(maxHist) + "\n";
    StrOut += "k\tIntensity\tHist\n";
    for(int k = 0; k < 65535 ;k++)
    {
        StrOut += to_string(k) + "\t" ;
        StrOut += to_string(k) + "\t" ;
        StrOut += to_string(HistA[k]) + "\t" ;
        StrOut += to_string(HistC[k]) + "\n" ;
    }
    //ui->textEdit->append(StrOut.c_str());
    path OutFileName = OutputDirectory;
    OutFileName.append("StackHistActinCalcein64K.txt");
    std::ofstream out2(OutFileName.string().c_str());
    out2 << StrOut;
    out2.close();
    StrOut.empty();



    delete[] HistA;
    delete[] HistC;

}

void MainWindow::on_spinBoxIntensityThreshold_valueChanged(int arg1)
{
    intensityThresholdIm1 = arg1;
    ImageAnalysis(ImIn, &FilePar1, intensityThresholdIm1);

    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_spinBoxIntensityThreshold2_valueChanged(int arg1)
{
    intensityThresholdIm2 = arg1;
    ImageAnalysis(ImIn2, &FilePar2, intensityThresholdIm2);

    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_checkBoxShowImageCombination_toggled(bool checked)
{
    showImageCombination = checked;
    //ShowImages();
    if(showImageCombination)
        ShowImages();
    else
        destroyWindow("Combination");
}

void MainWindow::on_pushButton2_2_clicked()
{
    QFileDialog dialog(this, "Open Folder");
    dialog.setFileMode(QFileDialog::Directory);
    //dialog.setDirectory("C:/Data/Sumona3Out/D28/20150819_28d_SC1_A2_Calc_PermOCN_ActinDAPI_1/");

    //QStringList FileList= dialog.e
    if(dialog.exec())
    {
        InputDirectoryIm1 = dialog.directory().path().toStdWString();
    }
    else
        return;
    OpenImage1Directory();

}

void MainWindow::on_pushButton2_3_clicked()
{
    QFileDialog dialog(this, "Open Folder");
    dialog.setFileMode(QFileDialog::Directory);
    //dialog.setDirectory("C:/Data/Sumona3Out/D28/20150819_28d_SC1_A2_Calc_PermOCN_ActinDAPI_1/");

    //QStringList FileList= dialog.e
    if(dialog.exec())
    {
        InputDirectoryIm2 = dialog.directory().path().toStdWString();
    }
    else
        return;

    OpenImage2Directory();
}

void MainWindow::on_pushButton2_4_clicked()
{
    QFileDialog dialog(this, "Open Folder");
    dialog.setFileMode(QFileDialog::Directory);
    //dialog.setDirectory("C:/Data/Sumona3Out/D28/20150819_28d_SC1_A2_Calc_PermOCN_ActinDAPI_1/");

    //QStringList FileList= dialog.e
    if(dialog.exec())
    {
        InputDirectoryIm3 = dialog.directory().path().toStdWString();
    }
    else
        return;

    OpenImage3Directory();
}

void MainWindow::on_FileIm3ListWidget_currentTextChanged(const QString &currentText)
{
    path imageToOpen = InputDirectoryIm3;//path imageToOpen = FilePar2.ImFolderName;

    imageToOpen.append(currentText.toStdWString());
    ImIn3 = imread(imageToOpen.string().c_str(),CV_LOAD_IMAGE_ANYDEPTH);
    if(ImIn.type() != CV_16U)
    {
        ImIn.convertTo(ImIn,CV_16U);
    }
    //medianBlur(ImIn2,ImIn2,3);
    //ImageAnalysis(ImIn2, &FilePar2, intensityThresholdIm2);
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_doubleSpinBoxImMin3_valueChanged(double arg1)
{
    minIm3 = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_doubleSpinBoxImMax3_valueChanged(double arg1)
{
    maxIm3 = arg1;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_FileIm1ListWidget_currentTextChanged(const QString &currentText)
{

}

void MainWindow::on_pushButtonLoadVectors_clicked()
{
    FreeImageVectors();
    if (!exists(InputDirectoryIm1))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm1.string()+ " not exists ").c_str());
        msgBox.exec();
        return;
    }
    if (!is_directory(InputDirectoryIm1))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm1.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        return;
    }
    for (directory_entry& FileToProcess : directory_iterator(InputDirectoryIm1))
    {
        regex FilePattern(ui->RegexImLineEdit->text().toStdString());
        if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
            continue;

        path PathLocal = FileToProcess.path();
        if (!exists(PathLocal))
        {
            QMessageBox msgBox;
            msgBox.setText((PathLocal.filename().string() + " File not exists" ).c_str());
            msgBox.exec();
            break;
        }
        Mat ImLocal;
        ImLocal = imread(PathLocal.string(), CV_LOAD_IMAGE_ANYDEPTH);
        if(ImIn.type() != CV_16U)
        {
            ImIn.convertTo(ImIn,CV_16U);
        }
        if (!ImLocal.empty())
            ImVect1.push_back(ImLocal);
    }
    int vect1Size = ImVect1.size();
    string OutStr = "Images 1 loaded: " + to_string(vect1Size);
    ui->textEdit->append(OutStr.c_str());


    bool imDirectory2OK = true;
    if (!exists(InputDirectoryIm2))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm2.string()+ " not exists ").c_str());
        msgBox.exec();
        imDirectory2OK = false;
    }
    if (!is_directory(InputDirectoryIm2))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm2.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        imDirectory2OK = false;
    }
    if(imDirectory2OK)
    {
        for (directory_entry& FileToProcess : directory_iterator(InputDirectoryIm2))
        {
            regex FilePattern(ui->RegexImLineEdit->text().toStdString());
            if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
                continue;

            path PathLocal = FileToProcess.path();
            if (!exists(PathLocal))
            {
                QMessageBox msgBox;
                msgBox.setText((PathLocal.filename().string() + " File not exists" ).c_str());
                msgBox.exec();
                break;
            }
            Mat ImLocal;
            ImLocal = imread(PathLocal.string(), CV_LOAD_IMAGE_ANYDEPTH);
            if(ImIn.type() != CV_16U)
            {
                ImIn.convertTo(ImIn,CV_16U);
            }
            if (!ImLocal.empty())
                ImVect2.push_back(ImLocal);
        }
        int vect2Size = ImVect2.size();
        OutStr = "Images 2 loaded: " + to_string(vect2Size);
    }
    else
    {
        OutStr = "Images 2 Directory invalid: ";
    }
    ui->textEdit->append(OutStr.c_str());

    bool imDirectory3OK = true;
    if (!exists(InputDirectoryIm3))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm3.string()+ " not exists ").c_str());
        msgBox.exec();
        imDirectory3OK = false;
    }
    if (!is_directory(InputDirectoryIm3))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectoryIm3.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        imDirectory3OK = false;
    }
    if(imDirectory3OK)
    {
        for (directory_entry& FileToProcess : directory_iterator(InputDirectoryIm3))
        {
            regex FilePattern(ui->RegexImLineEdit->text().toStdString());
            if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
                continue;

            path PathLocal = FileToProcess.path();
            if (!exists(PathLocal))
            {
                QMessageBox msgBox;
                msgBox.setText((PathLocal.filename().string() + " File not exists" ).c_str());
                msgBox.exec();
                break;
            }
            Mat ImLocal;
            ImLocal = imread(PathLocal.string(), CV_LOAD_IMAGE_ANYDEPTH);
            if(ImIn.type() != CV_16U)
            {
                ImIn.convertTo(ImIn,CV_16U);
            }
            if (!ImLocal.empty())
                ImVect3.push_back(ImLocal);
        }
        int vect3Size = ImVect3.size();
        OutStr = "Images 3 loaded: " + to_string(vect3Size);
    }
    else
    {
        OutStr = "Images 3 Directory invalid: ";
    }
    ui->textEdit->append(OutStr.c_str());



    bool imDirectory4OK = true;
    if (!exists(InputDirectory))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectory.string()+ " not exists ").c_str());
        msgBox.exec();
        imDirectory4OK = false;
    }
    if (!is_directory(InputDirectory))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectory.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        imDirectory4OK = false;
    }
    if(imDirectory4OK)
    {
        for (directory_entry& FileToProcess : directory_iterator(InputDirectory))
        {
            regex FilePattern(ui->RegexLineEdit->text().toStdString());
            if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
                continue;

            path PathLocal = FileToProcess.path();
            if (!exists(PathLocal))
            {
                QMessageBox msgBox;
                msgBox.setText((PathLocal.filename().string() + " File not exists" ).c_str());
                msgBox.exec();
                break;
            }
            FileParams ParamsLocal = GetDirectionData(PathLocal.string());

            if (ParamsLocal.ParamsVect.size())
                FileParVect1.push_back(ParamsLocal);
        }
        int vect4Size = FileParVect1.size();
        OutStr = "Directions 1 loaded: " + to_string(vect4Size);
    }
    else
    {
        OutStr = "Directions 1 directory  invalid: ";
    }
    ui->textEdit->append(OutStr.c_str());
    //

    bool imDirectory5OK = true;
    if (!exists(InputDirectory2))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectory2.string()+ " not exists ").c_str());
        msgBox.exec();
        imDirectory5OK = false;
    }
    if (!is_directory(InputDirectory2))
    {
        QMessageBox msgBox;
        msgBox.setText((InputDirectory2.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        imDirectory5OK = false;
    }
    if(imDirectory5OK)
    {
        for (directory_entry& FileToProcess : directory_iterator(InputDirectory2))
        {
            regex FilePattern(ui->RegexLineEdit->text().toStdString());
            if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
                continue;

            path PathLocal = FileToProcess.path();
            if (!exists(PathLocal))
            {
                QMessageBox msgBox;
                msgBox.setText((PathLocal.filename().string() + " File not exists" ).c_str());
                msgBox.exec();
                break;
            }
            FileParams ParamsLocal = GetDirectionData(PathLocal.string());

            if (ParamsLocal.ParamsVect.size())
                FileParVect2.push_back(ParamsLocal);
        }
        int vect5Size = FileParVect2.size();
        OutStr = "Directions 1 loaded: " + to_string(vect5Size);
    }
    else
    {
        OutStr = "Directions 1 directory  invalid: ";
    }
    ui->textEdit->append(OutStr.c_str());

    ui->spinBoxShowImVect1->setMaximum(vect1Size - 1);
    ui->spinBoxShowImVect1->setValue(0);
    ui->spinBoxYPlaneToShow->setMaximum(ImVect1[0].rows);
    ui->spinBoxYPlaneToShow->setValue(ImVect1[0].rows/2);
    vectSliceToShow = 0;

    //ShowImages();
    ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3);
    ShowXZFromVector(0);
}



void MainWindow::on_spinBoxShowImVect1_valueChanged(int arg1)
{
    vectSliceToShow = arg1;

    //ShowImages();
    ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3);
}

void MainWindow::on_spinBoxYPlaneToShow_valueChanged(int arg1)
{
    ShowXZFromVector(arg1);
}

void MainWindow::on_checkBoxShowVectIm1_toggled(bool checked)
{
    showVectIm1 = checked;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_checkBoxShowVectIm2_toggled(bool checked)
{
    showVectIm2 = checked;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_checkBoxShowVectIm3_toggled(bool checked)
{
    showVectIm3 = checked;
    ShowImages();
    //ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
    //               ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_spinBoxImOffsetVect_valueChanged(int arg1)
{
    vectSliceOffset = arg1;
    if(vectSliceOffset > 0)
    {
       ui->spinBoxShowImVect1->setMaximum(ImVect1.size() - vectSliceOffset - 1);
       ui->spinBoxShowImVect1->setMinimum(0);
    }
    if(vectSliceOffset == 0)
    {
        ui->spinBoxShowImVect1->setMaximum(ImVect1.size() - 1);
        ui->spinBoxShowImVect1->setMinimum(0);
    }
    if(vectSliceOffset < 0)
    {
       ui->spinBoxShowImVect1->setMinimum(0 - vectSliceOffset);
       ui->spinBoxShowImVect1->setMaximum(ImVect1.size() - 1);
    }
    vectSliceToShow = ui->spinBoxShowImVect1->value();


    //ui->spinBoxImOffsetVect->setV(vectSliceToShow);
    ShowFromVector(vectSliceToShow,vectSliceOffset,showVectIm1,showVectIm2,showVectIm3,
                   ui->checkBoxOffsetSecondIm->checkState());
}

void MainWindow::on_pushButton_2_clicked()
{
    path LocalDirectory;
    QFileDialog dialog(this, "Open Folder");
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory(InputDirectory.string().c_str());

    //QStringList FileList= dialog.e
    if(dialog.exec())
    {
        LocalDirectory = dialog.directory().path().toStdWString();
    }
    else
        return;

    InputDirectory = LocalDirectory;
    InputDirectory.append("/Actin/Direction/");
    InputDirectory2 = LocalDirectory;
    InputDirectory2.append("/Calcein/Direction/");
    InputDirectoryIm1 = LocalDirectory;
    InputDirectoryIm1.append("/Actin/");
    InputDirectoryIm2 = LocalDirectory;
    InputDirectoryIm2.append("/Calcein/");
    InputDirectoryIm3 = LocalDirectory;
    InputDirectoryIm3.append("/Nucleus/");

    OutputDirectory = LocalDirectory;
    if (!exists(OutputDirectory))
    {
        QMessageBox msgBox;
        msgBox.setText((OutputDirectory.string()+ " not exists ").c_str());
        msgBox.exec();
        InputDirectory = "C:\\";
    }
    if (!is_directory(OutputDirectory))
    {
        QMessageBox msgBox;
        msgBox.setText((OutputDirectory.string()+ " This is not a directory path ").c_str());
        msgBox.exec();
        OutputDirectory = "C:\\Data\\";
    }
    ui->LineEditSaveDirectory->setText(QString::fromWCharArray(OutputDirectory.wstring().c_str()));

    ui->textEdit->clear();
    OpenDirection1Directory();
    OpenDirection2Directory();
    OpenImage1Directory();
    OpenImage2Directory();
    OpenImage3Directory();
}

void MainWindow::on_spinBoxDisplayScale_valueChanged(int arg1)
{
    displayScale = arg1;
    ResizeImages(displayScale, ImIn.rows, ImIn.cols);
}

void MainWindow::on_comboBoxImageScale_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:
        imageScale = 4.0;
        break;
    case 1:
        imageScale = 2.0;
        break;
    case 2:
        imageScale = 1.0;
        break;

    case 3:
        imageScale = 1.0/2.0;
        break;
    case 4:
        imageScale = 1.0/4.0;
        break;
    default:
        imageScale = 1.0;
        break;
    }
    ShowImages();
}
