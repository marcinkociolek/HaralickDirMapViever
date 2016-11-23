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

#include "gradient.h"
#include "DispLib.h"

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
    string Line1,Line2;
    bool inputDirFound = 0;
    while (inFile1.good())
    {
        getline(inFile1, Line1,'\t');
        getline(inFile1, Line2);
        regex LinePattern("Input Directory 1:.+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            inputDirFound = 1;
            break;
        }
    }
    path ImFileName(Line2);
    while (inFile1.good())
    {
        getline(inFile1, Line1,'\t');
        getline(inFile1, Line2,'\t');
        regex LinePattern("In file - .+");
        if (regex_match(Line1.c_str(), LinePattern))
        {
            inputDirFound = 1;
            break;
        }
    }


}
