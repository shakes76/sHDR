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
#include <tclap/CmdLine.h> //Command line parser library

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMinimumImageFunction.h"
#include "itkHighDynamicRangeImageFilter.h"
//SMILI
#include "milxGlobal.h"
#include "milxFile.h"
#include "milxImage.h"

#include <cmath>
#include <iostream>

using namespace TCLAP;

//Globals

//Supported operations
enum operations { none = 0, tonemap, msde };

int main(int argc, char* argv[])
{
  const unsigned int dimension = 3;
  typedef float                 PixelType;
  typedef itk::Image< PixelType, 3 >    InputImageType;
  typedef itk::Image< PixelType, 3 > OutputImageType;

  //---------------------------
  ///Process Arguments
  CmdLine cmd("A tool for high dynamic range medical imaging", ' ', milx::NumberToString(0.2));

  ///Mandatory
  UnlabeledMultiArg<std::string> multinames("images", "Images to operate on (pixel type is auto detected from the first image)", true, "Images");
  ///Optional
  ValueArg<size_t> threadsArg("", "threads", "Set he number of global threads to use.", false, milx::NumberOfProcessors()/2, "Threads");
  ValueArg<std::string> outputArg("o", "output", "Output Image", false, "result.nii.gz", "Output");
  ValueArg<std::string> prefixArg("p", "prefix", "Output prefix for multiple output.", false, "img_", "Output Prefix");
  ValueArg<int> levelsArg("l", "levels", "Number of levels to use in the operation (such as MSDE).", false, 3, "Levels");
  ValueArg<float> sigmaRangeArg("r", "range", "Sigma Range (Stddev) value to operation (such as Tone Map or MSDE).", false, 0.08, "Sigma Range");
  ValueArg<float> sigmaDomainArg("d", "domain", "Sigma Domain (Stddev) value to operation (such as Tone Map or MSDE).", false, 8, "Sigma Domain");
  ValueArg<float> betaArg("b", "beta", "Beta value to operation (such as MSDE).", false, 0.8, "Beta");
  ValueArg<float> lambdaArg("", "lambda", "Lambda value to operation (such as MSDE).", false, 0.8, "Lambda");
  ValueArg<float> weightArg("w", "weight", "Weight value per image for operation (such as MSDE).", false, 0.8, "Weight");
  ValueArg<float> contrastArg("c", "contrast", "Contrast value per image for operation (such as Tone Map).", false, 5, "Contrast");
  ///Switches
  SwitchArg verboseMode("v", "verbose", "Verbose Output, i.e. output all intermediate results of the pipeline.", false);
  SwitchArg toneMapArg("t", "tone", "Apply tone mapping HDR mode to images.", false);
  SwitchArg msdeArg("m", "msde", "Apply MSDE HDR mode to images. Multiple channels/inputs required.", true);
  SwitchArg sosArg("", "sos", "Output sums of squares image. MSDE mode only.", false);
  SwitchArg aveArg("", "average", "Output average image. MSDE mode only.", false);

  ///Add argumnets
  cmd.add(multinames);
  cmd.add(threadsArg);
  cmd.add(outputArg);
  cmd.add(prefixArg);
  cmd.add(levelsArg);
  cmd.add(sigmaRangeArg);
  cmd.add(sigmaDomainArg);
  cmd.add(betaArg);
  cmd.add(lambdaArg);
  cmd.add(weightArg);
  cmd.add(contrastArg);
  cmd.add(verboseMode);
  cmd.add(toneMapArg);
  cmd.add(msdeArg);
  cmd.add(sosArg);
  cmd.add(aveArg);

  ///Parse the argv array.
  cmd.parse(argc, argv);

  ///Get the value parsed by each arg.
  //Filenames of surfaces
  const size_t threads = threadsArg.getValue();
  std::vector<std::string> filenames = multinames.getValue();
  std::string outputName = outputArg.getValue();
  const std::string outputPrefix = prefixArg.getValue();
  const int levels = levelsArg.getValue();
  float range = sigmaRangeArg.getValue();
  float domain = sigmaDomainArg.getValue();
  float beta = betaArg.getValue();
  float lambda = lambdaArg.getValue();
  float weight = weightArg.getValue();
  float contrast = contrastArg.getValue();

  std::cout << "Using levels: " << levels << std::endl;
  std::cout << "Using range and domain sigma as: " << range << ", " << domain << std::endl;
  std::cout << "Using beta and lambda values as: " << beta << ", " << lambda << std::endl;

  ///Setup ITK Threads
  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(threads);
  std::cout << "Threads to use: " << threads << std::endl;

  itk::HighDynamicRangeImageFilter<InputImageType, OutputImageType>::Pointer hdrImage = itk::HighDynamicRangeImageFilter<InputImageType, OutputImageType>::New();
    hdrImage->SetLevels(levels);
    hdrImage->SetBeta(beta);
    hdrImage->SetLambda(lambda);
    hdrImage->SetSigmaRange(range);
    hdrImage->SetSigmaDomain(domain);
    hdrImage->SetContrast(contrast);
  if(sosArg.isSet())
    hdrImage->SumsOfSquaresOn();
  if(aveArg.isSet())
    hdrImage->AverageOn();
    //hdrImage->BiasFieldOn();
  if(toneMapArg.isSet())
    hdrImage->ToneModeModeOn();
  if(msdeArg.isSet())
    hdrImage->MultiLightModeOn();
    hdrImage->SetNumberOfThreads(threads);
    //hdrImage->SetNumberOfIndexedInputs(filenames.size());

  //std::vector< itk::SmartPointer<InputImageType> > images;
  for (size_t j = 0; j < filenames.size(); j ++)
    {
      // load images
      std::cout << "Loading: " << filenames[j];
      InputImageType::Pointer image;
      milx::File::OpenImage<InputImageType>(filenames[j], image);
      //images.push_back(image); //prevent out-of-scope deletion

      hdrImage->AddInput(image);
      //hdrImage->SetInput(j, image);
      //hdrImage->PushBackInput(image);
      if(j == 0)
        hdrImage->AddInputWeight(1.0);
      else
        hdrImage->AddInputWeight(weight);
      std::cout << " with weight " << hdrImage->GetInputWeight(j) << std::endl;
    }

  try
    {
      std::cout << "Applying HDR filter ..." << std::endl;
      hdrImage->Update();
      std::cout << "Done" << std::endl;
    }
  catch (itk::ExceptionObject& e)
    {
      std::cerr << "Exception detected: "  << e.GetDescription();
      return EXIT_FAILURE;
    }

  if(verboseMode.isSet())
    {
      std::cout << "Debug Output" << std::endl;
      std::vector<OutputImageType::Pointer> levelResults = hdrImage->GetMultiLightResults();
      std::vector<OutputImageType::Pointer> diffResults = hdrImage->GetMultiLightDetails();
      for(size_t level = 0; level < levels; level ++)
        {
          std::string filename = outputPrefix + "_bilateral_level_" + milx::NumberToString(level) + ".nii.gz";
          milx::File::SaveImage<OutputImageType>(filename, levelResults[level]);
          std::string filenameDiff = outputPrefix + "_diff_level_" + milx::NumberToString(level) + ".nii.gz";
          milx::File::SaveImage<OutputImageType>(filenameDiff, diffResults[level]);
        }

      for (int j = 0; j < filenames.size(); j ++)
        {
          std::string filename = outputPrefix + "_image_" + milx::NumberToString(j) + "_base.nii.gz";
          milx::File::SaveImage<OutputImageType>(filename, hdrImage->GetLevelBaseImage(j));
          std::string filenameDiff = outputPrefix + "_image_" + milx::NumberToString(j) + "_details.nii.gz";
          milx::File::SaveImage<OutputImageType>(filenameDiff, hdrImage->GetLevelDetailImage(j));
        }
    }

  std::cout << "Write Output" << std::endl;
  std::string filename = outputPrefix + "_final_base_" + ".nii.gz";
  milx::File::SaveImage<OutputImageType>(filename, hdrImage->GetBaseImage());
  filename = outputPrefix + "_final_detail_" + ".nii.gz";
  milx::File::SaveImage<OutputImageType>(filename, hdrImage->GetDetailImage());
  if(sosArg.isSet())
  {
    filename = outputPrefix + "_sos.nii.gz";
    milx::File::SaveImage<OutputImageType>(filename, hdrImage->GetSumsOfSquaresImage());
  }
  if(aveArg.isSet())
  {
    filename = outputPrefix + "_average.nii.gz";
    milx::File::SaveImage<OutputImageType>(filename, hdrImage->GetAverageImage());
  }
  //filename = outputPrefix + "_biasfield.nii.gz";
  //milx::File::SaveImage<OutputImageType>(filename, hdrImage->GetBiasFieldImage());
  filename = outputPrefix + ".nii.gz";
  milx::File::SaveImage<OutputImageType>(filename, hdrImage->GetOutput());

  std::cout << "Complete" << std::endl;
  return EXIT_SUCCESS;
}
