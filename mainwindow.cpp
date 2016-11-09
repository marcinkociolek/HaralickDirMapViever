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
}
