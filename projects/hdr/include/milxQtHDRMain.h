#ifndef MILXQTHDRMAIN_H
#define MILXQTHDRMAIN_H
//ITK
#include <itkImage.h>
//milxQt
#include "milxQtMain.h"

class MILXQT_EXPORT milxQtHDRMain : public milxQtMain
{
    Q_OBJECT

public:
    /*!
        \fn milxQtHDRMain::milxQtHDRMain(QWidget *parent = 0)
        \brief The standard constructor
    */
    milxQtHDRMain(QWidget *theParent = 0);
    /*!
        \fn milxQtHDRMain::~milxQtMain()
        \brief The standard destructor
    */
    virtual ~milxQtHDRMain();

    void hdr();

public slots:
    void startWizard();
    void showImageFileDialog();
    void showAdvancedOptions(int state);

protected:
    //Toolbar actions
    QAction *wizardAction;

    //Toolbars
    QToolBar *hdrToolBar; //!< Some actions from file menu

    //FAI variables for wizard
    QWizard wizard;
    QListWidget *lstFilenames;
    QLineEdit *txtLevels;
    QLineEdit *txtSigmaRange;
    QLineEdit *txtSigmaSpatial;
    QLineEdit *txtBeta;
    QLineEdit *txtLambda;
    QLineEdit *txtThreads;
    QCheckBox *chkVerbose;
    QCheckBox *chkAdvanced;

    //data
    QStringList filenames;

    /*!
        \fn milxQtHDRMain::contextMenuEvent(QContextMenuEvent *event)
        \brief The context menu setup member
    */
    void contextMenuEvent(QContextMenuEvent *event);
    /*!
        \fn milxQtHDRMain::createWizard()
        \brief Create the wizard for application
    */
    void createWizard();
    /*!
        \fn milxQtHDRMain::createActions()
        \brief Create the actions for context menu etc.
    */
    void createActions();
    /*!
        \fn milxQtHDRMain::createToolBars()
        \brief Creates the toolbar for the main window.
    */
    void createToolBars();
    void createConnections();
};

#endif // MILXQTHDRMAIN_H
