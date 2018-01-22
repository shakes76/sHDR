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
#include "itkMinimumImageFunction.h"
#include "itkHighDynamicRangeImageFilter.h"

#include "milxGlobal.h"
#include "milxFile.h"
#include "milxImage.h"

#include <cmath>
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

//Globals
const bool debugOutput = true;

int main(int argc, char* argv[])
{
  const unsigned int dimension = 3;
  typedef float                 PixelType;
  typedef itk::Image< PixelType, 3 >    InputImageType;
  typedef itk::Image< PixelType, 3 > OutputImageType;

  if(argc < 7)
    {
    std::cerr << "Compute the Multiscale Shape and Detail Enhancement (MSDE) of images\n";
    std::cerr << "Usage: " << argv[0] << " OutputPrefix sigmaRange sigmaDomain levels InputImage1 InputImage2 ... \n";
    return EXIT_FAILURE;
    }

  std::string outputPrefix = argv[1];
  float range = atof(argv[2]);
  float domain = atof(argv[3]);
  int levels = atoi(argv[4]);
  std::vector<std::string> filenames;
  for(int j = 5; j < argc; j ++)
    filenames.push_back(argv[j]);

  std::cout << "Using levels: " << levels << std::endl;
  std::cout << "Using range and domain sigma as: " << range << ", " << domain << std::endl;

//  float epsilon = 1e-8; //avoid divide by zero
  float lambdaValue = 0.8; //HDR paramter
  std::vector<OutputImageType::Pointer> bases;
  std::vector<OutputImageType::Pointer> details;
  for (size_t j = 0; j < filenames.size(); j ++)
    {
      // load images
      InputImageType::Pointer image;
      milx::File::OpenImage<InputImageType>(filenames[j], image);
      InputImageType::RegionType region = image->GetLargestPossibleRegion();
      std::cout << "Size of image read: " << region.GetSize()[0] << ", " << region.GetSize()[1] << ", " << region.GetSize()[2] << std::endl;

      ///run MLIC
      itk::HighDynamicRangeImageFilter<InputImageType, OutputImageType>::Pointer hdrImage = itk::HighDynamicRangeImageFilter<InputImageType, OutputImageType>::New();
      hdrImage->CreateMultiLightImageCollection(image, range, domain, levels);
      std::vector<OutputImageType::Pointer> levelResults = hdrImage->GetMultiLightResults();
      std::vector<OutputImageType::Pointer> diffResults = hdrImage->GetMultiLightDetails();

      //Debug, check MLIC output
      for(size_t level = 0; level < levels; level ++)
        {
          std::string filename = outputPrefix + "_bilateral_level_" + milx::NumberToString(level) + ".nii.gz";
          milx::File::SaveImage<OutputImageType>(filename, levelResults[level]);
          std::string filenameDiff = outputPrefix + "_diff_level_" + milx::NumberToString(level) + ".nii.gz";
          milx::File::SaveImage<OutputImageType>(filenameDiff, diffResults[level]);
        }

      hdrImage->ComputeMultiscaleShapeDetailEnhancement(levelResults, diffResults, region, levels, lambdaValue);

      /*InputImageType::Pointer levelDetailBlank = milx::Image<OutputImageType>::BlankImage(0.0, region.GetSize());
      InputImageType::Pointer levelDetail = milx::Image<OutputImageType>::MatchInformation(levelDetailBlank, image); //ensure images in same space
      InputImageType::Pointer levelBase;
      for(size_t level = 0; level < levels; level ++)
        {
          std::cout << "\tProcessing image in level " << level << std::endl;
          OutputImageType::Pointer weightsBlank = milx::Image<OutputImageType>::BlankImage(0.0, region.GetSize());
          OutputImageType::Pointer weights = milx::Image<OutputImageType>::MatchInformation(weightsBlank, image); //ensure images in same space

          OutputImageType::Pointer gradMagResult = milx::Image<OutputImageType>::GradientMagnitude(levelResults[level]);
          std::string filename = outputPrefix + "_grad_level_" + milx::NumberToString(level) + ".nii.gz";
          if(debugOutput)
            milx::File::SaveImage<OutputImageType>(filename, gradMagResult);

          typedef itk::MinimumImageFunction<OutputImageType> FilterType;
          FilterType::Pointer minImageFunction = FilterType::New();
          minImageFunction->SetInputImage(levelResults[level]);

          std::cout << "Weights: " << std::endl;
          itk::ImageRegionIteratorWithIndex<OutputImageType> resultIterator(levelResults[level], region);
          itk::ImageRegionIteratorWithIndex<OutputImageType> diffIterator(diffResults[level], region);
          itk::ImageRegionIteratorWithIndex<OutputImageType> gradIterator(gradMagResult, region);
          itk::ImageRegionIteratorWithIndex<OutputImageType> weightsIterator(weights, region);
          while(!diffIterator.IsAtEnd())
              {
                PixelType minValue = static_cast<PixelType>(minImageFunction->EvaluateAtIndex(resultIterator.GetIndex()));
                PixelType C = gradIterator.Get()/(minValue+epsilon); //penalise strong edges which the ideal bilateral would not have picked up
                // Set the current detail pixel
                PixelType U = exp( abs(diffIterator.Get())-C );
                weightsIterator.Set(U);

                //reduce ratio between min max values
                diffIterator.Set( copysign( pow(fabs(diffIterator.Get()), lambdaValue), diffIterator.Get()) ); //copysign - Return x with the sign of y

                ++resultIterator;
                ++diffIterator;
                ++gradIterator;
                ++weightsIterator;
              }

          filename = outputPrefix + "_weights_level_" + milx::NumberToString(level) + ".nii.gz";
          if(debugOutput)
              milx::File::SaveImage<OutputImageType>(filename, weights);

          //Smooth weights
          OutputImageType::Pointer weightsFinal = milx::Image<OutputImageType>::GaussianSmooth(weights, 1); //smooth, 8 parameter from Fattal et al. 2007

          filename = outputPrefix + "_weights_final_level_" + milx::NumberToString(level) + ".nii.gz";
          if(debugOutput)
              milx::File::SaveImage<OutputImageType>(filename, weightsFinal);

          //Muliply, Add and Deep Copy detail
          itk::ImageRegionConstIterator<OutputImageType> inputIterator(diffResults[level], region);
          itk::ImageRegionConstIterator<OutputImageType> weightIterator(weightsFinal, region);
          itk::ImageRegionIterator<OutputImageType> outputIterator(levelDetail, region);
          while(!inputIterator.IsAtEnd())
            {
                outputIterator.Set(inputIterator.Get()*weightIterator.Get()+outputIterator.Get());
                ++inputIterator;
                ++weightIterator;
                ++outputIterator;
            }

          filename = outputPrefix + "_detail_level_" + milx::NumberToString(level) + ".nii.gz";
          if(debugOutput)
              milx::File::SaveImage<OutputImageType>(filename, levelDetail);

          if (level == levelResults.size() - 1) //last one
            levelBase = levelResults[level];
        }*/
      std::string filename = outputPrefix + "_image_" + milx::NumberToString(j) + "_base.nii.gz";
      milx::File::SaveImage<OutputImageType>(filename, hdrImage->GetBaseImage());
      std::string filenameDiff = outputPrefix + "_image_" + milx::NumberToString(j) + "_details.nii.gz";
      milx::File::SaveImage<OutputImageType>(filenameDiff, hdrImage->GetDetailImage());
    }

  std::cout << "Complete" << std::endl;
  return EXIT_SUCCESS;
}
