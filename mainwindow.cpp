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
}

MainWindow::~MainWindow()
{
    delete ui;
}
//----------------------------------------------------------------------------------------------------------------
FileParams GetDirectionData(path FileToOpen)
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
void MainWindow::ProcessImage()
{


    if (!exists(FileToOpen))
        return;

    std::ifstream inFile1(FileToOpen.string());
    if (!inFile1.is_open())
    {
        QMessageBox msgBox;
        msgBox.setText((FileToOpen.filename().string() + " File not exists" ).c_str());
        msgBox.exec();
        return;
    }
// read params frm file
    string Line1,Line2;
    bool inputDirFound = 0;
    //read input directory
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

    ImFileName = Line2;
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

    tileShape = stoi(Line1.substr(12,1));

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
    maxTileX = stoi(Line1.substr(13));
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
    maxTileY = stoi(Line1.substr(13));
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
    shiftTileX = stoi(Line1.substr(14));
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
    shiftTileY = stoi(Line1.substr(14));

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
    offsetTileX = stoi(Line1.substr(15));

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
    offsetTileY = stoi(Line1.substr(15));


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
    ImFileName.append(Line1.substr(10));


//end of read params
    ui->textEdit->clear();
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
    int ValueCount = 0;
    size_t stringPos = 0;
    while(1)
    {
        stringPos = Line1.find("\t",stringPos);
        ValueCount++;
        if(stringPos != string::npos)
            break;
        stringPos++;
    }
    // read feature names
    std::stringstream InStringStream(Line1);

    std::string subStr;

    NamesVector.empty();
    char FeatName[256];
    while(InStringStream.good())
    {
        InStringStream.getline(FeatName,250,'\t');
        NamesVector.push_back(FeatName);
    }
    ui->labelFeatName->setText(NamesVector[ui->spinBoxFeatureToShow->value()+2].c_str());

    ui->textEdit->append("\n");
    ui->textEdit->append(ItoStrLZ(ValueCount,2).c_str());
    ui->textEdit->append("\n");

    //list<int> TilesParams;
    vector<TileParams> ParamsVect;
//read directionalities
    while(inFile1.good())
    {
        TileParams params;
        getline(inFile1,Line1);
        params.FromString(Line1);
        if(params.tileX > -1 && params.tileY > -1)
            ParamsVect.push_back(params);//TilesParams.push_back(stoi(Line1));
    }

    inFile1.close();

    ui->textEdit->append(ImFileName.string().c_str());
    ImIn = imread(ImFileName.string().c_str(),CV_LOAD_IMAGE_ANYDEPTH);
    maxX = ImIn.cols;
    maxY = ImIn.rows;

    if(!maxX || ! maxY)
    {
        QMessageBox msgBox;
        ui->textEdit->append("\n Image File not exists");
        return;
    }

    if(ui->checkBoxShowSudoColor->checkState())
        ImShow = ShowImage16PseudoColor(ImIn,ui->doubleSpinBoxImMin->value(),ui->doubleSpinBoxImMax->value());
    else
        ImShow = ImIn;

    tileLineThickness = ui->spinBoxImposedShapeThickness->value();

    if(ui->checkBoxShowSape->checkState())
    {
        switch (tileShape)
        {
        case 1:
            for (int y = offsetTileY; y <= (maxY - offsetTileY); y += shiftTileY)
            {
                for (int x = offsetTileX; x <= (maxX - offsetTileX); x += shiftTileX)
                {
                    rectangle(ImShow, Point(x - maxTileX / 2, y - maxTileY / 2),
                        Point(x - maxTileX / 2 + maxTileX - 1, y - maxTileY / 2 + maxTileY - 1),
                        Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                }
            }
            break;
        case 2:
            for (int y = offsetTileY; y <= (maxY - offsetTileY); y += shiftTileY)
            {
                for (int x = offsetTileX; x <= (maxX - offsetTileX); x += shiftTileX)
                {
                    ellipse(ImShow, Point(x, y),
                        Size(maxTileX / 2, maxTileY / 2), 0.0, 0.0, 360.0,
                        Scalar(0.0, 0.0, 0.0, 0.0), tileLineThickness);
                }
            }
            break;
        case 3:
            for (int y = offsetTileY; y <= (maxY - offsetTileY); y += shiftTileY)
            {
                for (int x = offsetTileX; x <= (maxX - offsetTileX); x += shiftTileX)
                {
                    int edgeLength = maxTileX;
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
    int numOfDirections = ParamsVect.size();

    for(int i = 0; i < numOfDirections; i++)
    {
        int x  = ParamsVect[i].tileX * shiftTileX + offsetTileX;
        int y  = ParamsVect[i].tileY * shiftTileY + offsetTileY;
        double angle = ParamsVect[i].Params[ui->spinBoxFeatureToShow->value()];
        double lineLength = ui->spinBoxLineLength->value();
        int imposedLineThickness = ui->spinBoxImposedLineThickness->value();
/*
        if (ProcOptions.lineLengthPropToConfidence)
            lineLength = (double)(ProcOptions.lineHalfLength) / (ProcOptions.maxOfset - ProcOptions.minOfset + 1) / featCount * maxAngleCombVot;
        else
            lineLength = (double)(ProcOptions.lineHalfLength);
*/
        int lineOffsetX = (int)round(lineLength * sin(angle * PI / 180.0));
        int lineOffsetY = (int)round(lineLength * cos(angle * PI / 180.0));

        if (angle >= -600 && ui->checkBoxShowLine->checkState() && ParamsVect[i].Params[9] >= ui->doubleSpinBoxProcTresh->value() )
        {
            //line(ImToShow, Point(barCenterX - lineOffsetX, barCenterY - lineOffsetY), Point(barCenterX + lineOffsetX, barCenterY + lineOffsetY), Scalar(0, 0.0, 0.0, 0.0), ProcOptions.imposedLineThickness);
            line(ImShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0, 0.0, 0.0, 0.0), imposedLineThickness);
        }
    }


    /*
    for(list<int>::iterator iterTileY =TilesX.begin(); iterTileY != TileY.end(); iterTileY++)
        double lineLength;
        if (ProcOptions.lineLengthPropToConfidence)
            lineLength = (double)(ProcOptions.lineHalfLength) / (ProcOptions.maxOfset - ProcOptions.minOfset + 1) / featCount * maxAngleCombVot;
        else
            lineLength = (double)(ProcOptions.lineHalfLength);
        int lineOffsetX = (int)round(lineLength *  sin((double)(bestAngleCombVot)*ProcOptions.angleStep* PI / 180.0));
        int lineOffsetY = (int)round(lineLength * cos((double)(bestAngleCombVot)*ProcOptions.angleStep* PI / 180.0));

        if (maxAngleCombVot >= ProcOptions.minHit)
        {
            //line(ImToShow, Point(barCenterX - lineOffsetX, barCenterY - lineOffsetY), Point(barCenterX + lineOffsetX, barCenterY + lineOffsetY), Scalar(0, 0.0, 0.0, 0.0), ProcOptions.imposedLineThickness);
            line(ImToShow, Point(x - lineOffsetX, y - lineOffsetY), Point(x + lineOffsetX, y + lineOffsetY), Scalar(0, 0.0, 0.0, 0.0), ProcOptions.imposedLineThickness);
        }
    */
    imshow("Input image",ImShow);
}

void MainWindow::on_pushButton_clicked()
{
    QFileDialog dialog(this, "Open Folder");
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory("C:/Data/Sumona3Out/D28/20150819_28d_SC1_A2_Calc_PermOCN_ActinDAPI_1/");

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
}

void MainWindow::on_FileListWidget_currentTextChanged(const QString &currentText)
{
    FileToOpen = InputDirectory;
    FileToOpen.append(currentText.toStdWString());
    ProcessImage();

}

void MainWindow::on_checkBoxShowSape_toggled(bool checked)
{
    ProcessImage();
}

void MainWindow::on_checkBoxShowLine_toggled(bool checked)
{
    ProcessImage();
}

void MainWindow::on_checkBoxShowSudoColor_toggled(bool checked)
{
    ProcessImage();
}

void MainWindow::on_spinBoxImposedShapeThickness_valueChanged(int arg1)
{
    ProcessImage();
}

void MainWindow::on_spinBoxImposedLineThickness_valueChanged(int arg1)
{
    ProcessImage();
}

void MainWindow::on_spinBoxLineLength_valueChanged(int arg1)
{
    ProcessImage();
}

void MainWindow::on_spinBoxFeatureToShow_valueChanged(int arg1)
{
    ProcessImage();
}

void MainWindow::on_pushButtonChoseOutDir_clicked()
{
    QFileDialog dialog(this, "Open Folder");
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory("C:/Data");

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
    if(!maxX || ! maxY)
    {
        return;
    }
    path OutFileName = OutputDirectory;
    OutFileName.append(ImFileName.stem().string() + ".bmp");
    imwrite(OutFileName.string(),ImShow);
}

void MainWindow::on_doubleSpinBoxImMin_valueChanged(double arg1)
{
    ProcessImage();
}

void MainWindow::on_doubleSpinBoxImMax_valueChanged(double arg1)
{
   ProcessImage();
}

void MainWindow::on_doubleSpinBoxProcTresh_valueChanged(double arg1)
{
   ProcessImage();
}
