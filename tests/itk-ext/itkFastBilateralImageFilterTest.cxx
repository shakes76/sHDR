#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif
#include <fstream>
#include "itkFastBilateralImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkPNGImageIO.h"
#include "itkPNGImageIOFactory.h"
#include "itkImageRegionIterator.h"


/**
 * This test was originally taken from the tests for the itkBilateralImageFilter
 * and modified for the itkFastBilateralImageFilter. Modified by Shakes Chandra for parameters.
 * Note that I found that Crash on Windows if O2 optimization is enabled
 */
int main(int ac, char* av[] )
{
  if(ac < 5)
    {
    std::cerr << "Usage: " << av[0] << " InputImage sigmaRange sigmaDomain OutputImage\n";
    return -1;
    }

  float range = atof(av[2]);
  float domain = atof(av[3]);

  typedef float PixelType;
  typedef itk::Image<PixelType, 3> myImage;
  itk::ImageFileReader<myImage>::Pointer input 
    = itk::ImageFileReader<myImage>::New();
  input->SetFileName(av[1]);
  
  // Create a filter
  typedef itk::FastBilateralImageFilter<myImage,myImage> FilterType;

  FilterType::Pointer filter1 = FilterType::New();
    filter1->SetInput(input->GetOutput());
    filter1->SetRangeSigma(range);
    filter1->SetDomainSigma(domain);
  try
    {
    input->Update();
    std::cout << "Using range and domain sigma as: " << range << ", " << domain << std::endl;
    myImage::RegionType region = input->GetOutput()->GetLargestPossibleRegion();
    std::cout << "Size of image read: " << region.GetSize()[0] << ", " << region.GetSize()[1] << ", " << region.GetSize()[2] << std::endl;

    std::cout << "Applying ..." << std::endl;
    filter1->Update();
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return -1;
    }

  // Generate test image
  itk::ImageFileWriter<myImage>::Pointer writer;
    writer = itk::ImageFileWriter<myImage>::New();
    writer->SetInput( filter1->GetOutput() );
    writer->SetFileName( av[4] );
    writer->Update();
  std::cout << "Complete" << std::endl;

  return EXIT_SUCCESS;
}
