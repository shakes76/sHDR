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
//#include "itkFastBilateralImageFilter.h"
#include "itkHighDynamicRangeImageFilter.h"

#include "milxGlobal.h"
#include "milxFile.h"

#include <iostream>

//class ShowProgressObject
//{
//public:
//  ShowProgressObject(itk::ProcessObject* o)
//    {m_Process = o;}
//  void ShowProgress()
//    {std::cout << "Progress " << m_Process->GetProgress() << std::endl;}
//  itk::ProcessObject::Pointer m_Process;
//};

int main(int argc, char* argv[])
{
  const unsigned int dimension = 3;
  typedef float                 PixelType;
  typedef itk::Image< PixelType, 3 >    InputImageType;
  typedef itk::Image< PixelType, 3 > OutputImageType;

  if(argc < 6)
    {
    std::cerr << "Compute the Multi-Light Image Collection\n";
    std::cerr << "Usage: " << argv[0] << " InputImage sigmaRange sigmaDomain levels OutputPrefix\n";
    return EXIT_FAILURE;
    }

  float range = atof(argv[2]);
  float domain = atof(argv[3]);
  int levels = atoi(argv[4]);
  std::string outputPrefix = argv[5];

  std::cout << "Using levels: " << levels << std::endl;
  std::cout << "Using range and domain sigma as: " << range << ", " << domain << std::endl;

  // load images
  InputImageType::Pointer image;
  milx::File::OpenImage<InputImageType>(argv[1], image);
  InputImageType::RegionType region = image->GetLargestPossibleRegion();
  std::cout << "Size of image read: " << region.GetSize()[0] << ", " << region.GetSize()[1] << ", " << region.GetSize()[2] << std::endl;

  ///run MLIC
  itk::HighDynamicRangeImageFilter<InputImageType, OutputImageType>::Pointer hdrImage = itk::HighDynamicRangeImageFilter<InputImageType, OutputImageType>::New();
  hdrImage->CreateMultiLightImageCollection(image, range, domain, levels);
  std::vector<OutputImageType::Pointer> levelResults = hdrImage->GetMultiLightResults();
  std::vector<OutputImageType::Pointer> diffResults = hdrImage->GetMultiLightDetails();
  for(size_t level = 0; level < levels; level ++)
    {
      std::string filename = outputPrefix + "_bilateral_level_" + milx::NumberToString(level) + ".nii.gz";
      milx::File::SaveImage<OutputImageType>(filename, levelResults[level]);
      std::string filenameDiff = outputPrefix + "_diff_level_" + milx::NumberToString(level) + ".nii.gz";
      milx::File::SaveImage<OutputImageType>(filenameDiff, diffResults[level]);
    }

  std::cout << "Complete" << std::endl;
  return EXIT_SUCCESS;
}
