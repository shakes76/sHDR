
#include "milxFile.h"
#include "milxImage.h"

#include "milxQtHDRMain.h"
#include "milxQtHDRImage.h"

#include "itkHighDynamicRangeImageFilter.h"

//stl
#include <math.h>
//ITK
#if (ITK_VERSION_MAJOR > 3)
    #include <vnl/algo/vnl_symmetric_eigensystem.h>
    #include <vnl/algo/vnl_real_eigensystem.h>
    #include <vnl/vnl_inverse.h>
    #include <vnl/vnl_real.h>
    #include <vnl/algo/vnl_svd.h>
    #include <vnl/vnl_cross.h>
#else
    #include <vxl/core/vnl/algo/vnl_symmetric_eigensystem.h>
    #include <vxl/core/vnl/algo/vnl_real_eigensystem.h>
    #include <vxl/core/vnl/vnl_inverse.h>
    #include <vxl/core/vnl/vnl_real.h>
    #include <vxl/core/vnl/algo/vnl_svd.h>
    #include <vxl/core/vnl/vnl_cross.h>
#endif
#include <itkTranslationTransform.h>
#include <itkCastImageFilter.h>
//VTK
#include <vtkSphereSource.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>

milxQtHDRMain::milxQtHDRMain(QWidget *theParent) : milxQtMain(theParent)
{
    defaultViewBox->setCurrentIndex(1); //force view to coronal
    defaultViewTypeBox->setCurrentIndex(0); //force singe view

    createWizard();

    createActions();

    createToolBars();

    createConnections();
}

milxQtHDRMain::~milxQtHDRMain()
{

    printDebug("HDR Main Destroyed.");
}

void milxQtHDRMain::contextMenuEvent(QContextMenuEvent *currentEvent)
{
    milxQtMain::contextMenuEvent(currentEvent);
}

void milxQtHDRMain::showImageFileDialog()
{
    QSettings settings("Shekhar Chandra", "milxQt");
    //Add supported file entry
    QString exts = openMedImageExts.c_str();
    QString path = settings.value("recentPath").toString();

    QFileDialog *fileOpener = new QFileDialog;
    QStringList names = fileOpener->getOpenFileNames(&wizard,
                            tr("Select Images to Open"),
                            path,
                            tr(exts.toStdString().c_str()) ); //!< \todo Check and validate extensions support at Open in Main class

    foreach(QString name, names)
        lstFilenames->addItem(name);
}

void milxQtHDRMain::showAdvancedOptions(int state)
{
  if(state == Qt::Checked)
  {
      txtLevels->setDisabled(true);
      txtSigmaRange->setDisabled(true);
      txtSigmaSpatial->setDisabled(true);
      txtLambda->setDisabled(true);
      txtThreads->setDisabled(true);
  }
  else
  {
      txtLevels->setDisabled(false);
      txtSigmaRange->setDisabled(false);
      txtSigmaSpatial->setDisabled(false);
      txtLambda->setDisabled(false);
      txtThreads->setDisabled(false);
  }
}


void milxQtHDRMain::startWizard()
{
    //use wizaed to ask
    wizard.restart();

    int ret = wizard.exec();

    if(ret == QDialog::Rejected)
    {
        wizard.restart();
        return;
    }

    if(lstFilenames->count() == 0)
    {
        printError("File list was empty. Try again");
        wizard.restart();
        return;
    }

    if(workspaces->count() > 1)
        newTab();

    for(int j = 0; j < lstFilenames->count(); j ++)
    {
        printInfo("Adding: " + lstFilenames->item(j)->text());
        filenames.push_back(lstFilenames->item(j)->text());
    }

    qApp->processEvents();

    hdr();

    //close docks
    console->dockWidget()->hide();
}

void milxQtHDRMain::hdr()
{
    printInfo("Computing HDR of images");

    emit working(-1);

    const size_t threads = txtThreads->text().toUInt();
    const size_t levels = txtLevels->text().toUInt();
    const float range = txtSigmaRange->text().toFloat();
    const float domain = txtSigmaSpatial->text().toFloat();
    const float beta = txtBeta->text().toFloat();
    const float lambda = txtLambda->text().toFloat();
    const float weight = 1.0;

    printInfo("Using levels: " + QString::number(levels));
    printInfo("Using range and domain sigma as: " + QString::number(range) + ", " + QString::number(domain));
    printInfo("Using beta and lambda values as: " + QString::number(beta) + ", " + QString::number(lambda));

    ///Setup ITK Threads
    itk::MultiThreader::SetGlobalDefaultNumberOfThreads(threads);
    printInfo("Threads to use: " + QString::number(threads));

    itk::HighDynamicRangeImageFilter<floatImageType, floatImageType>::Pointer hdrImage = itk::HighDynamicRangeImageFilter<floatImageType, floatImageType>::New();
        hdrImage->SetLevels(levels);
        hdrImage->SetBeta(beta);
        hdrImage->SetLambda(lambda);
        hdrImage->SetSigmaRange(range);
        hdrImage->SetSigmaDomain(domain);
        hdrImage->SumsOfSquaresOn();
        hdrImage->AverageOn();
        //hdrImage->BiasFieldOn();
        hdrImage->MultiLightModeOn();
        hdrImage->SetNumberOfThreads(threads);
        hdrImage->AddObserver(itk::ProgressEvent(), milx::ProgressUpdates);

    foreach(QString filename, filenames)
    {
        // load images
        printInfo("Loading: " + filename);

        floatImageType::Pointer image;
        //Load image
        milx::File::OpenImage<floatImageType>(filename.toStdString(), image);
        qApp->processEvents();

        //Normalise to 0-1
        printInfo("Normalising Image to 0-1");
        floatImageType::Pointer imageNorm = milx::Image<floatImageType>::RescaleIntensity(image, 0.0, 1.0);

        hdrImage->AddInput(imageNorm);
        hdrImage->AddInputWeight(weight);
        //printInfo(" with weight " + QString::number(hdrImage->GetInputWeight(j)));
        qApp->processEvents();
    }

    try
    {
        printInfo("Applying HDR filter ...");
        hdrImage->Update();
        qApp->processEvents();
    }
    catch (itk::ExceptionObject& e)
    {
        printError("Exception detected: " + QString(e.GetDescription()));
        emit done(-1);
        return;
    }

    //Display images
    if(!chkVerbose->isChecked())
    {
        QPointer<milxQtImage> imgBase = new milxQtImage;  //list deletion
        imgBase->setData(hdrImage->GetBaseImage());
        imgBase->setName("Base Image");
        imgBase->setConsole(console);
        imgBase->generateImage();
        predisplay(imgBase);

        QPointer<milxQtImage> imgDetail = new milxQtImage;  //list deletion
        imgDetail->setData(hdrImage->GetDetailImage());
        imgDetail->setName("Detail Image");
        imgDetail->setConsole(console);
        imgDetail->generateImage();
        predisplay(imgDetail);

        QPointer<milxQtImage> imgSOS = new milxQtImage;  //list deletion
        imgSOS->setData(hdrImage->GetSumsOfSquaresImage());
        imgSOS->setName("SoS Image");
        imgSOS->setConsole(console);
        imgSOS->generateImage();
        predisplay(imgSOS);

        QPointer<milxQtImage> imgAve = new milxQtImage;  //list deletion
        imgAve->setData(hdrImage->GetAverageImage());
        imgAve->setName("Average Image");
        imgAve->setConsole(console);
        imgAve->generateImage();
        predisplay(imgAve);
    }

    QPointer<milxQtImage> imgHDR = new milxQtImage;  //list deletion
    imgHDR->setData(hdrImage->GetOutput());
    imgHDR->setName("HDR Image");
    imgHDR->setConsole(console);
    imgHDR->generateImage();
    predisplay(imgHDR);
    
    qApp->processEvents();
    milxQtMain::tileTabVertically();
    printInfo("Done.");
    emit done(-1);
}

void milxQtHDRMain::createWizard()
{
  //intro
  QWizardPage *introPage = new QWizardPage;
  introPage->setTitle("Introduction");

  QLabel *label1 = new QLabel("This wizard will help you apply High Dynamic Range algorithms to MRI images.");
  label1->setWordWrap(true);
  QLabel *label2 = new QLabel("You will be asked to provide input channel/sequence images to compute the algorithm.");
  label2->setWordWrap(true);
  QLabel *label3 = new QLabel("You will then be asked to provide the parameters and the choice of algorithm to apply to the images.");
  label3->setWordWrap(true);
  QVBoxLayout *introLayout = new QVBoxLayout;
  introLayout->addWidget(label1);
  introLayout->addWidget(label2);
  introLayout->addWidget(label3);
  introPage->setLayout(introLayout);

  //ask for image
  QWizardPage *imgPage = new QWizardPage;
  imgPage->setTitle("Images");

  QLabel *label4 = new QLabel("Please provide the MR images to be used for synthesis. Images are assumed to be co-registered.");
  label4->setWordWrap(true);
  lstFilenames = new QListWidget;
  QPushButton *btnImageName = new QPushButton;
  connect(btnImageName, SIGNAL(clicked()), this, SLOT(showImageFileDialog()));
  btnImageName->setText("Browse...");
  QHBoxLayout *imgNameLayout = new QHBoxLayout;
  imgNameLayout->addWidget(lstFilenames);
  imgNameLayout->addWidget(btnImageName);
  QGroupBox *imgGroupBox = new QGroupBox("Image Filenames");
  imgGroupBox->setLayout(imgNameLayout);
  QVBoxLayout *imgLayout = new QVBoxLayout;
  imgLayout->addWidget(label4);
  imgLayout->addWidget(imgGroupBox);
  imgPage->setLayout(imgLayout);

  //ask for mesh
  QWizardPage *paraPage = new QWizardPage;
  paraPage->setTitle("Algorithm Parameters");

  QLabel *label5 = new QLabel("Please provide the algorithm and parameters.");
  QLabel *label5b = new QLabel("Beta factor (0.0-1.0) controls the detail, where 1.0 is maximum detail. Default is 0.8.");
  QLabel *label5c = new QLabel("Sigma parameters control the scale of the details to be separated.");
  label5->setWordWrap(true);
  label5b->setWordWrap(true);
  label5c->setWordWrap(true);
  QFormLayout *paraFormLayout = new QFormLayout;
  QFormLayout *paraAdvFormLayout = new QFormLayout;
  txtLevels = new QLineEdit;
  txtLevels->setText("3");
  txtLevels->setValidator( new QIntValidator(1, 10, this) );
  txtSigmaRange = new QLineEdit;
  txtSigmaRange->setText("0.04");
  txtSigmaSpatial = new QLineEdit;
  txtSigmaSpatial->setText("4");
  txtBeta = new QLineEdit;
  txtBeta->setText("0.8");
  txtBeta->setValidator( new QDoubleValidator(0, 1.0, 3, this) );
  txtLambda = new QLineEdit;
  txtLambda->setText("0.8");
  txtLambda->setValidator( new QDoubleValidator(0, 1.0, 3, this) );
  txtThreads = new QLineEdit;
  txtThreads->setText(QString::number(milx::NumberOfProcessors()/2));
  txtThreads->setValidator( new QIntValidator(1, milx::NumberOfProcessors(), this) );
  chkVerbose = new QCheckBox;
  chkAdvanced = new QCheckBox;
  connect(chkAdvanced, SIGNAL(stateChanged(int)), this, SLOT(showAdvancedOptions(int)));
  paraAdvFormLayout->addRow(tr("&Levels:"), txtLevels);
  paraAdvFormLayout->addRow(tr("&Sigma Range/Color:"), txtSigmaRange);
  paraAdvFormLayout->addRow(tr("&Sigma Spatial:"), txtSigmaSpatial);
  paraAdvFormLayout->addRow(tr("&Lambda:"), txtLambda);
  paraAdvFormLayout->addRow(tr("&Threads:"), txtThreads);
  paraFormLayout->addRow(tr("&Beta:"), txtBeta);
  paraFormLayout->addRow(tr("HDR Image Only"), chkVerbose);
  paraFormLayout->addRow(tr("Advanced Options"), chkAdvanced);
  chkVerbose->setChecked(true);
  chkAdvanced->setChecked(true);
  QGroupBox *paraGroupBox = new QGroupBox;
  paraGroupBox->setLayout(paraAdvFormLayout);
  QVBoxLayout *meshLayout = new QVBoxLayout;
  meshLayout->addWidget(label5);
  meshLayout->addWidget(label5b);
  meshLayout->addWidget(label5c);
  meshLayout->addLayout(paraFormLayout);
  meshLayout->addWidget(paraGroupBox);
  paraPage->setLayout(meshLayout);

  //wizard class var
  wizard.addPage(introPage);
  wizard.addPage(imgPage);
  wizard.addPage(paraPage);
  wizard.setWindowTitle("HDR Wizard");
}

void milxQtHDRMain::createActions()
{
    wizardAction = new QAction(this);
    wizardAction->setText(QApplication::translate("Main", "HDR Wizard", 0, QApplication::UnicodeUTF8));
}

void milxQtHDRMain::createToolBars()
{
    hdrToolBar = addToolBar(tr("HDR"));
    hdrToolBar->addAction(wizardAction);
}

void milxQtHDRMain::createConnections()
{
    connect(wizardAction, SIGNAL(activated()), this, SLOT(startWizard()));
}
