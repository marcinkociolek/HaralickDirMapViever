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
    featNr = ui->spinBoxFeatureToShow->value();
    meanIntensityTreshold = ui->doubleSpinBoxProcTresh->value();
    lineLength = ui->spinBoxLineLength->value();
    imposedLineThickness = ui->spinBoxImposedLineThickness->value();
    showFirstImage = ui->checkBoxShowFirstIm->checkState();
    showSecondImage = ui->checkBoxShowSecondIm->checkState();
    showTwoImages = ui->checkBoxShowTwoIm->checkState();
    minIm2 = ui->doubleSpinBoxImMin2->value();
    maxIm2 = ui->doubleSpinBoxImMax2->value();
    meanIntensityTreshold2 = ui->doubleSpinBoxProcTresh2->value();
    zOffset = ui->spinBoxZOffset->value();
    zFrame = 0;
    InputDirectory = "C:/Data/Sumona3Out/";

    intensityThresholdIm1 = ui->spinBoxIntensityThreshold->value();
    intensityThresholdIm2 = ui->spinBoxIntensityThreshold2->value();
}

MainWindow::~MainWindow()
{
    delete ui;
}
//----------------------------------------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------------------------------
DirDetectionParams MainWindow::GetDirectionData2(path FileToOpen)
{
    DirDetectionParams LocalParams;
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
//----------------------------------------------------------------------------------------------------------------
//FileParams MainWindow::GetStatisticalDataFromImage(path FileToOpen)

//----------------------------------------------------------------------------------------------------------------
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
                           std::string ShowWindowName)

{
    int maxX = Im.cols;
    int maxY = Im.rows;

    if(!maxX || ! maxY)
    {
        QMessageBox msgBox;
        ui->textEdit->append("\n Image File not exists");
        return;
    }
    Mat ImShow;
    Im.convertTo(Im,CV_16U);

    if(sudocolor)
        ImShow = ShowImage16PseudoColor(Im,minIm,maxIm);
    else
        ImShow = Im;

    if(showShape)
    {
        switch (Params.tileShape)
        {
        case 1:
            for (int y = Params.offsetTileY; y <= (maxY - Params.offsetTileY); y += Params.shiftTileY)
            {
                for (int x = Params.offsetTileX; x <= (maxX - Params.offsetTileX); x += Params.shiftTileX)
                {
                    rectangle(ImShow, Point(x - Params.maxTileX / 2, y - Params.maxTileY / 2),
                        Point(x - Params.maxTileX / 2 + Params.maxTileX - 1, y - Params.maxTileY / 2 + Params.maxTileY - 1),
                        Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                }
            }
            break;
        case 2:
            for (int y = Params.offsetTileY; y <= (maxY - Params.offsetTileY); y += Params.shiftTileY)
            {
                for (int x = Params.offsetTileX; x <= (maxX - Params.offsetTileX); x += Params.shiftTileX)
                {
                    ellipse(ImShow, Point(x, y),
                        Size(Params.maxTileX / 2, Params.maxTileY / 2), 0.0, 0.0, 360.0,
                        Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                }
            }
            break;
        case 3:
            for (int y = Params.offsetTileY; y <= (maxY - Params.offsetTileY); y += Params.shiftTileY)
            {
                for (int x = Params.offsetTileX; x <= (maxX - Params.offsetTileX); x += Params.shiftTileX)
                {
                    int edgeLength = Params.maxTileX;
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
            }
            break;
        default:
            break;
        }

    }
    int numOfDirections = (int)Params.ParamsVect.size();

    for(int i = 0; i < numOfDirections; i++)
    {
        int x  = Params.ParamsVect[i].tileX * Params.shiftTileX + Params.offsetTileX;
        int y  = Params.ParamsVect[i].tileY * Params.shiftTileY + Params.offsetTileY;
        double angle = Params.ParamsVect[i].Params[featNr];

        int lineOffsetX = (int)round(lineLength * sin(angle * PI / 180.0));
        int lineOffsetY = (int)round(lineLength * cos(angle * PI / 180.0));

        if (angle >= -600 && showLine && Params.ParamsVect[i].Params[9] >= meanIntensityTreshold )
        {
            line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0, 0.0, 0.0, 0.0), imposedLineThickness);
        }
    }
    imshow(ShowWindowName.c_str(),ImShow);
}
//----------------------------------------------------------------------------------------------------------------
void MainWindow::ImageAnalysis(cv::Mat Im, FileParams *Params, unsigned short intensityThreshold)
{
    Im.convertTo(Im,CV_16U);
    Mat Roi;
    int roiMaxX, roiMaxY; // Bounding box sizes for ROI
    switch (Params->tileShape) // Different tile shapes
    {
    case 1: // Rectangle
        roiMaxX = Params->maxTileX;
        roiMaxY = Params->maxTileY;
        Roi = Mat::ones(roiMaxY, roiMaxX, CV_16U);
        break;
    case 2: // Ellipse
        roiMaxX = Params->maxTileX;
        roiMaxY = Params->maxTileY;
        Roi = Mat::zeros(roiMaxY, roiMaxX, CV_16U);
        ellipse(Roi, Point(roiMaxX / 2, roiMaxY / 2),
            Size(roiMaxX / 2, roiMaxY / 2), 0.0, 0.0, 360.0,
            1, -1);
        break;
    case 3: // Hexagon
    {
        int edgeLength = Params->maxTileX;
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
        int x  = Params->ParamsVect[i].tileX * Params->shiftTileX + Params->offsetTileX;
        int y  = Params->ParamsVect[i].tileY * Params->shiftTileY + Params->offsetTileY;

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
        Params->ParamsVect[i].Params[9] = percentage;
    }
}

//----------------------------------------------------------------------------------------------------------------
void MainWindow::Show2Image(cv::Mat Im, cv::Mat Im2, FileParams Params, FileParams Params2,
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
    int maxX = Im.cols;
    int maxY = Im.rows;

    if(!maxX || ! maxY)
    {
        QMessageBox msgBox;
        ui->textEdit->append("\n Image File not exists");
        return;
    }
    Mat ImShow;
    int maxX2 = Im2.cols;
    int maxY2 = Im2.rows;

    if(!maxX2 || ! maxY2)
    {
        QMessageBox msgBox;
        ui->textEdit->append("\n Image File not exists");
        return;
    }
    Mat ImShow2;
    Im.convertTo(Im,CV_16U);
    Im2.convertTo(Im2,CV_16U);
    if(sudocolor)
    {
        ImShow = ShowImage16PseudoColor(Im,minIm,maxIm);
        ImShow2 = ShowImage16PseudoColor(Im2,minIm2,maxIm2);
    }
    else
    {
        ImShow = Im;
        ImShow2 = Im2;
    }

    if(showShape)
    {
        switch (Params.tileShape)
        {
        case 1:
            for (int y = Params.offsetTileY; y <= (maxY - Params.offsetTileY); y += Params.shiftTileY)
            {
                for (int x = Params.offsetTileX; x <= (maxX - Params.offsetTileX); x += Params.shiftTileX)
                {
                    rectangle(ImShow, Point(x - Params.maxTileX / 2, y - Params.maxTileY / 2),
                        Point(x - Params.maxTileX / 2 + Params.maxTileX - 1, y - Params.maxTileY / 2 + Params.maxTileY - 1),
                        Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                    rectangle(ImShow2, Point(x - Params.maxTileX / 2, y - Params.maxTileY / 2),
                        Point(x - Params.maxTileX / 2 + Params.maxTileX - 1, y - Params.maxTileY / 2 + Params.maxTileY - 1),
                        Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                }
            }
            break;
        case 2:
            for (int y = Params.offsetTileY; y <= (maxY - Params.offsetTileY); y += Params.shiftTileY)
            {
                for (int x = Params.offsetTileX; x <= (maxX - Params.offsetTileX); x += Params.shiftTileX)
                {
                    ellipse(ImShow, Point(x, y),
                        Size(Params.maxTileX / 2, Params.maxTileY / 2), 0.0, 0.0, 360.0,
                        Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                    ellipse(ImShow2, Point(x, y),
                        Size(Params.maxTileX / 2, Params.maxTileY / 2), 0.0, 0.0, 360.0,
                        Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                }
            }
            break;
        case 3:
            for (int y = Params.offsetTileY; y <= (maxY - Params.offsetTileY); y += Params.shiftTileY)
            {
                for (int x = Params.offsetTileX; x <= (maxX - Params.offsetTileX); x += Params.shiftTileX)
                {
                    int edgeLength = Params.maxTileX;
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

                    line(ImShow2, vertice0, vertice1, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                    line(ImShow2, vertice1, vertice2, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                    line(ImShow2, vertice2, vertice3, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                    line(ImShow2, vertice3, vertice4, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                    line(ImShow2, vertice4, vertice5, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                    line(ImShow2, vertice5, vertice0, Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                }
            }
            break;
        default:
            break;
        }

    }
    int numOfDirections = (int)Params.ParamsVect.size();

    for(int i = 0; i < numOfDirections; i++)
    {
        int x  = Params.ParamsVect[i].tileX * Params.shiftTileX + Params.offsetTileX;
        int y  = Params.ParamsVect[i].tileY * Params.shiftTileY + Params.offsetTileY;
        double angle = Params.ParamsVect[i].Params[featNr];

        int lineOffsetX = (int)round(lineLength * sin(angle * PI / 180.0));
        int lineOffsetY = (int)round(lineLength * cos(angle * PI / 180.0));

        if (angle >= -600 && showLine && Params.ParamsVect[i].Params[9] >= meanIntensityTreshold )
        {
            line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0, 0.0, 0.0, 0.0), imposedLineThickness);
            line(ImShow2, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0, 0.0, 0.0, 0.0), imposedLineThickness);
        }
    }
    numOfDirections = (int)Params2.ParamsVect.size();

    for(int i = 0; i < numOfDirections; i++)
    {
        int x  = Params2.ParamsVect[i].tileX * Params2.shiftTileX + Params2.offsetTileX;
        int y  = Params2.ParamsVect[i].tileY * Params2.shiftTileY + Params2.offsetTileY;
        double angle = Params2.ParamsVect[i].Params[featNr];

        int lineOffsetX = (int)round(lineLength * sin(angle * PI / 180.0));
        int lineOffsetY = (int)round(lineLength * cos(angle * PI / 180.0));

        if (angle >= -600 && showLine && Params2.ParamsVect[i].Params[9] >= meanIntensityTreshold2 )
        {
            line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(250.0, 0.0, 0.0, 0.0), imposedLineThickness);
            line(ImShow2, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(250.0, 0.0, 0.0, 0.0), imposedLineThickness);
        }
    }

    //imshow("Input image 1",ImShow);
    //imshow("Input image 2",ImShow2);

    Mat ImOut = Mat::zeros(Size(maxX + maxX2,maxY) ,ImShow.type());
    ImShow.copyTo(ImOut(Rect(0, 0, ImShow.cols , ImShow.rows)));
    ImShow2.copyTo(ImOut(Rect(maxX, 0, ImShow2.cols , ImShow2.rows)));

    imshow("Two images",ImOut);
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowImages()
{
    if(showFirstImage)
        ShowImage(ImIn, FilePar1, sudocolor, showShape, showLine, minIm, maxIm, tileLineThickness, featNr,
                  meanIntensityTreshold, lineLength, imposedLineThickness, "In File 1");
    if(showSecondImage)
        ShowImage(ImIn2, FilePar2, sudocolor, showShape, showLine, minIm2, maxIm2, tileLineThickness, featNr,
                  meanIntensityTreshold2, lineLength, imposedLineThickness, "In File 2");
    if(showTwoImages)
        Show2Image(ImIn, ImIn2, FilePar1, FilePar2, sudocolor, showShape, showLine, minIm, maxIm, minIm2, maxIm2,
                   tileLineThickness, featNr, meanIntensityTreshold, meanIntensityTreshold2, lineLength, imposedLineThickness);

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

void MainWindow::on_FileListWidget_currentTextChanged(const QString &currentText)
{
    FileToOpen = InputDirectory;
    FileToOpen.append(currentText.toStdWString());
    FilePar1 = GetDirectionData(FileToOpen);
    ImIn = imread(FilePar1.ImFileName.string().c_str(),CV_LOAD_IMAGE_ANYDEPTH);
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
}

void MainWindow::on_checkBoxShowSudoColor_toggled(bool checked)
{
    sudocolor = checked;
    ShowImages();
}

void MainWindow::on_spinBoxImposedShapeThickness_valueChanged(int arg1)
{
    tileLineThickness = arg1;
    ShowImages();
}

void MainWindow::on_spinBoxImposedLineThickness_valueChanged(int arg1)
{
    imposedLineThickness = arg1;
    ShowImages();
}

void MainWindow::on_spinBoxLineLength_valueChanged(int arg1)
{
    lineLength = arg1;
    ShowImages();
}

void MainWindow::on_spinBoxFeatureToShow_valueChanged(int arg1)
{
    featNr = arg1;
    ShowImages();
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
    path OutFileName = OutputDirectory;
    OutFileName.append(ImFileName.stem().string() + ".bmp");
    imwrite(OutFileName.string(),ImShow);
}

void MainWindow::on_doubleSpinBoxImMin_valueChanged(double arg1)
{
    minIm = arg1;
    ShowImages();
}

void MainWindow::on_doubleSpinBoxImMax_valueChanged(double arg1)
{
    maxIm = arg1;
    ShowImages();
}

void MainWindow::on_doubleSpinBoxProcTresh_valueChanged(double arg1)
{
    meanIntensityTreshold = arg1;
    ShowImages();
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

void MainWindow::on_File2ListWidget_currentTextChanged(const QString &currentText)
{
    FileToOpen2 = InputDirectory2;
    FileToOpen2.append(currentText.toStdWString());
    FilePar2 = GetDirectionData(FileToOpen2);
    ImIn2 = imread(FilePar2.ImFileName.string().c_str(),CV_LOAD_IMAGE_ANYDEPTH);
    medianBlur(ImIn2,ImIn2,3);
    ImageAnalysis(ImIn2, &FilePar2, intensityThresholdIm2);
    ShowImages();
    //ProcessImage();

}

void MainWindow::on_doubleSpinBoxImMin2_valueChanged(double arg1)
{
    minIm2 = arg1;
    ShowImages();
}

void MainWindow::on_doubleSpinBoxImMax2_valueChanged(double arg1)
{
    maxIm2 = arg1;
    ShowImages();
}

void MainWindow::on_doubleSpinBoxProcTresh2_valueChanged(double arg1)
{
    meanIntensityTreshold2 = arg1;
    medianBlur(ImIn,ImIn,3);
    ImageAnalysis(ImIn, &FilePar1, intensityThresholdIm1);
    ShowImages();
}

void MainWindow::on_checkBoxShowSecondIm_toggled(bool checked)
{
    showSecondImage = checked;
    if(showSecondImage)
        ShowImage(ImIn2, FilePar2, sudocolor, showShape, showLine, minIm2, maxIm2, tileLineThickness, featNr,
                  meanIntensityTreshold2, lineLength, imposedLineThickness, "In File 2");
    else
        destroyWindow("In File 2");
}


void MainWindow::on_checkBoxShowFirstIm_toggled(bool checked)
{
    showFirstImage = checked;
    if(showFirstImage)
        ShowImage(ImIn, FilePar1, sudocolor, showShape, showLine, minIm, maxIm, tileLineThickness, featNr,
                  meanIntensityTreshold, lineLength, imposedLineThickness, "In File 1");
    else
        destroyWindow("In File 1");
}

void MainWindow::on_checkBoxShowTwoIm_toggled(bool checked)
{
    showTwoImages = checked;
    if(showTwoImages)
        Show2Image(ImIn, ImIn2, FilePar1, FilePar2, sudocolor, showShape, showLine, minIm, maxIm, minIm2, maxIm2,
                   tileLineThickness, featNr, meanIntensityTreshold, meanIntensityTreshold2, lineLength, imposedLineThickness);
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
    for(int zOffset = -10;zOffset <=10; zOffset++)
    {
        string StrOut = "Actin File Name\tCalcein File Name\t z Plane\ttile Y\ttile X\tdir Actin\tdir Calcein\tmean intensity Actin\tmean intensity Calcein\tAbs dir difference\n";

        int start1;
        int stop1;
        int rowCount = ui->FileListWidget->count();

        start1 = 0;
        if((start1 + zOffset)< 0)
            start1 = 0 - zOffset;

        stop1 = rowCount;
        if((stop1 + zOffset)>= rowCount)
            stop1 = rowCount - zOffset;




        for(int k = start1; k < stop1 ;k++)
        {
            path LocalFileToOpen;

            LocalFileToOpen  = InputDirectory;
            LocalFileToOpen.append(ui->FileListWidget->item(k)->text().toStdWString());
            FileParams Params1 = GetDirectionData(LocalFileToOpen);
            ImIn = imread(Params1.ImFileName.string().c_str(),CV_LOAD_IMAGE_ANYDEPTH);
            medianBlur(ImIn,ImIn,3);
            ImageAnalysis(ImIn, &Params1, intensityThresholdIm1);


            LocalFileToOpen  = InputDirectory2;
            LocalFileToOpen.append(ui->File2ListWidget->item(k+zOffset)->text().toStdWString());
            FileParams Params2 = GetDirectionData(LocalFileToOpen);
            ImIn = imread(Params2.ImFileName.string().c_str(),CV_LOAD_IMAGE_ANYDEPTH);
            medianBlur(ImIn,ImIn,3);
            ImageAnalysis(ImIn, &Params2, intensityThresholdIm1);

            int numOfDirections = Params1.ParamsVect.size();
            for(int i = 0; i < numOfDirections; i++)
            {
                int x  = Params1.ParamsVect[i].tileX;
                int y  = Params1.ParamsVect[i].tileY;

                float angle1 = Params1.ParamsVect[i].Params[featNr];
                float angle2 = Params2.ParamsVect[i].Params[featNr];
                float meanInt1 = Params1.ParamsVect[i].Params[9];
                float meanInt2 = Params2.ParamsVect[i].Params[9];

                if (angle1 >= -600 && angle2 >= -600 && meanInt1 >= meanIntensityTreshold && meanInt2 >= meanIntensityTreshold2 )
                {
                    float diff;
                    if(angle1 > angle2)
                        diff = angle1 - angle2;
                    else
                        diff = angle2 - angle1;

                    if (diff > 90)
                        diff = 180-diff;

                    StrOut += ui->FileListWidget->item(k)->text().toStdString() + "\t";
                    StrOut += ui->File2ListWidget->item(k)->text().toStdString() + "\t";
                    StrOut += to_string(k) + "\t";
                    StrOut += to_string(y) + "\t";
                    StrOut += to_string(x) + "\t";
                    StrOut += to_string(angle1) + "\t";
                    StrOut += to_string(angle2) + "\t";
                    StrOut += to_string(meanInt1) + "\t";
                    StrOut += to_string(meanInt2) + "\t";
                    StrOut += to_string(diff) + "\n";
                }
            }


        }
        //ui->textEdit->append(StrOut.c_str());
        path OutFileName = OutputDirectory;
        OutFileName.append("Summary ofset" + ItoStrLZPlusSign(zOffset,2) + ".txt");
        std::ofstream out(OutFileName.string().c_str());
        out << StrOut;
        out.close();
        StrOut.empty();
    }


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
        path ImFile = Params.ImFileName;
        string FileName = ImFile.string();
        if(!exists(ImFile))
            continue;
        Mat ImLocal = imread(FileName,CV_LOAD_IMAGE_ANYDEPTH);

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
        path ImFile = Params.ImFileName;
        string FileName = ImFile.string();

        if(!exists(ImFile))
            continue;
        Mat ImLocal = imread(FileName,CV_LOAD_IMAGE_ANYDEPTH);
        ImLocal.convertTo(ImLocal,CV_16U);
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
        path ImFile = Params.ImFileName;
        string FileName = ImFile.string();

        if(!exists(ImFile))
            continue;
        Mat ImLocal = imread(FileName,CV_LOAD_IMAGE_ANYDEPTH);
        ImLocal.convertTo(ImLocal,CV_16U);
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
}

void MainWindow::on_spinBoxIntensityThreshold2_valueChanged(int arg1)
{
    intensityThresholdIm2 = arg1;
    ImageAnalysis(ImIn2, &FilePar2, intensityThresholdIm2);

    ShowImages();
}
