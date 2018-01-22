#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif
#include <fstream>
#include "itkMinimumImageFunction.h"
#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"


/**
 * This test was originally taken from the tests for the itkBilateralImageFilter
 * and modified for the itkFastBilateralImageFilter. Modified by Shakes Chandra for parameters.
 * Note that I found that Crash on Windows if O2 optimization is enabled
 */
int main(int ac, char* av[] )
{
  if(ac < 3)
    {
    std::cerr << "Print the minimum of index provided\n";
    std::cerr << "Usage: " << av[0] << " InputImage Index\n";
    return EXIT_FAILURE;
    }

  int indexValue = atoi(av[2]);

  typedef float PixelType;
  typedef itk::Image<PixelType, 3> myImage;
  itk::ImageFileReader<myImage>::Pointer input 
    = itk::ImageFileReader<myImage>::New();
  input->SetFileName(av[1]);

  try
    {
    input->Update();
    myImage::RegionType region = input->GetOutput()->GetLargestPossibleRegion();
    std::cout << "Size of image read: " << region.GetSize()[0] << ", " << region.GetSize()[1] << ", " << region.GetSize()[2] << std::endl;
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return EXIT_FAILURE;
    }
  
  // Create a filter
  typedef itk::MinimumImageFunction<myImage> FilterType;

  FilterType::Pointer medianImageFunction = FilterType::New();
    medianImageFunction->SetInputImage(input->GetOutput());

  itk::Index<3> index;
    index.Fill(indexValue);
  std::cout << "Minimum at " << index << " is " << static_cast<PixelType>(medianImageFunction->EvaluateAtIndex(index)) << std::endl;

  std::cout << "Complete" << std::endl;

  return EXIT_SUCCESS;
}
