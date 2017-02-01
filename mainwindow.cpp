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

using namespace boost;
using namespace std;

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

void MainWindow::on_pushButton_clicked()
{
    QFileDialog dialog(this, "Open Folder");
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory("E:/Ziarno/14.04.2016 Rozne klasy/Dobre");

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
    if (!exists(FileToOpen))
        return;

    std::ifstream inFile1(FileToOpen.string());
    if (!inFile1.is_open())
    {
        QMessageBox msgBox;
        msgBox.setText((FileToOpen.filename().string() + " File not exists" ).c_str());
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
    path ImFileName(Line2);
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
    ui->textEdit->append("\n");
    ui->textEdit->append(ItoStrLZ(ValueCount,2).c_str());
    ui->textEdit->append("\n");

    list<string> TilesParams;

//read directionalities
    while (inFile1.good())
    {
        getline(inFile1, Line1,'\t');
        TilesParams.push_back(stoi(Line1);
    }



    ui->textEdit->append(ImFileName.string().c_str());
    ImIn = imread(ImFileName.string().c_str(),CV_LOAD_IMAGE_ANYDEPTH);
    maxX = ImIn.cols;
    maxY = ImIn.rows;

    ImShow = ShowImage16PseudoColor(ImIn,0.0,64000.0);

    tileLineThickness = 1;

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
