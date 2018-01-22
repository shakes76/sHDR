/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"

#include <iostream>

class ShowProgressObject
{
public:
  ShowProgressObject(itk::ProcessObject* o)
    {m_Process = o;}
  void ShowProgress()
    {std::cout << "Progress " << m_Process->GetProgress() << std::endl;}
  itk::ProcessObject::Pointer m_Process;
};

int main(int argc, char* argv[])
{
  const unsigned int dimension = 3;
  typedef float                 PixelType;
  typedef itk::Image< PixelType, 3 >    InputImageType;
  typedef itk::Image< PixelType, 3 > OutputImageType;

  if(argc < 4)
    {
    std::cerr << "Preprocess image for HDR. Processing such as rescale intensities etc." << std::endl;
    std::cerr << "Usage: " << argv[0] << " InputImage Trim? OutputImage\n";
    return EXIT_FAILURE;
    }

  int trim = atoi(argv[2]);

  // load images
  itk::ImageFileReader<InputImageType>::Pointer input1 = itk::ImageFileReader<InputImageType>::New();
  input1->SetFileName(argv[1]);

  try
    {
    input1->Update();
    InputImageType::RegionType region = input1->GetOutput()->GetLargestPossibleRegion();
    std::cout << "Size of image read: " << region.GetSize()[0] << ", " << region.GetSize()[1] << ", " << region.GetSize()[2] << std::endl;
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return EXIT_FAILURE;
    }

  // create the filter
  typedef itk::RescaleIntensityImageFilter<InputImageType,InputImageType> FilterType;
  FilterType::Pointer filter1 = FilterType::New();
    filter1->SetInput(input1->GetOutput());
    filter1->SetOutputMaximum(1.0);
    filter1->SetOutputMinimum(0.0);
  try
    {
    std::cout << "Applying ..." << std::endl;
    filter1->Update();
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return EXIT_FAILURE;
    }

  // Generate test image
  itk::ImageFileWriter<InputImageType>::Pointer writer;
    writer = itk::ImageFileWriter<InputImageType>::New();
    writer->SetInput( filter1->GetOutput() );
    writer->SetFileName( argv[3] );
    writer->Update();

  std::cout << "Complete" << std::endl;
  return EXIT_SUCCESS;
}
