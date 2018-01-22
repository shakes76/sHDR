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
#ifndef itkHighDynamicRangeImageFilter_hxx
#define itkHighDynamicRangeImageFilter_hxx

#include "itkHighDynamicRangeImageFilter.h"
#include "itkFastBilateralImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkLogImageFilter.h"
#include "itkProgressReporter.h"
#include "itkImageAlgorithm.h"

#include "itkMinimumImageFunction.h"

#include "milxImage.h"
#include "milxFile.h"

namespace itk
{
template< typename TInputImage, typename TOutputImage >
HighDynamicRangeImageFilter< TInputImage, TOutputImage >
::HighDynamicRangeImageFilter()
{
//  m_Spacing = 1.0;
//  m_Origin = 0.0;
  m_Levels = 3;
  m_SumsOfSquares = false;
  m_Average = false;
  m_Beta = 0.8;
  m_Lambda = 0.8;
  m_SigmaRange = 4;
  m_SigmaDomain = 20;
  m_Contrast = 5;
  m_Mode = ToneMap;
}

template< typename TInputImage, typename TOutputImage >
void
HighDynamicRangeImageFilter< TInputImage, TOutputImage >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

//  os << indent << "Spacing: " << m_Spacing << std::endl;
//  os << indent << "Origin: " << m_Origin << std::endl;
}

template< typename TInputImage, typename TOutputImage >
void
HighDynamicRangeImageFilter< TInputImage, TOutputImage >
::GenerateData()
{
  if(m_Mode == MultiLight)
  {
    for(IndexValueType idx = 0; idx < this->GetNumberOfInputs(); ++idx)
    {
      this->InvokeEvent( ProgressEvent() );

      typename InputImageType::Pointer image = const_cast<InputImageType *>(this->GetInput(idx));

      if(!image)
        std::cout << "Warning: Image from Input is NULL" << std::endl;

      typename InputImageType::RegionType region = image->GetLargestPossibleRegion();
      std::cout << "Size of image " << idx << ": " << region.GetSize()[0] << ", " << region.GetSize()[1] << ", " << region.GetSize()[2] << std::endl;

      ///run MLIC
      CreateMultiLightImageCollection(image, m_SigmaRange, m_SigmaDomain, m_Levels);

      //Debug, check MLIC output
      //      for(size_t level = 0; level < m_Levels; level ++)
      //        {
      //          std::string filename = outputPrefix + "_bilateral_level_" + milx::NumberToString(level) + ".nii.gz";
      //          milx::File::SaveImage<OutputImageType>(filename, m_LevelResults[level]);
      //          std::string filenameDiff = outputPrefix + "_diff_level_" + milx::NumberToString(level) + ".nii.gz";
      //          milx::File::SaveImage<OutputImageType>(filenameDiff, m_DiffResults[level]);
      //        }

      ComputeMultiscaleShapeDetailEnhancement(m_LevelResults, m_DiffResults, region, m_Levels, m_Lambda);

      //      std::string filename = outputPrefix + "_image_" + milx::NumberToString(idx) + "_base.nii.gz";
      //      milx::File::SaveImage<OutputImageType>(filename, m_LevelBaseImage);
      //      std::string filenameDiff = outputPrefix + "_image_" + milx::NumberToString(idx) + "_details.nii.gz";
      //      milx::File::SaveImage<OutputImageType>(filenameDiff, m_LevelDetailImage);

      m_LevelBaseImages.push_back(m_BaseImage);
      m_LevelDetailImages.push_back(m_DetailImage);
    }

    typename InputImageType::Pointer imageFirst = const_cast<InputImageType *>(this->GetInput(0));
    typename InputImageType::RegionType region = imageFirst->GetLargestPossibleRegion();
    typename TOutputImage::Pointer baseBlank = milx::Image<TOutputImage>::BlankImage(0.0, region.GetSize());
    typename TOutputImage::Pointer base = milx::Image<TOutputImage>::MatchInformation(baseBlank, imageFirst); //ensure images in same space
    typename TOutputImage::Pointer detailBlank = milx::Image<TOutputImage>::BlankImage(0.0, region.GetSize());
    typename TOutputImage::Pointer detail = milx::Image<TOutputImage>::MatchInformation(detailBlank, imageFirst); //ensure images in same space
    std::cout << "Synthesize layers ... " << std::endl;
    for(IndexValueType idx = 0; idx < this->GetNumberOfInputs(); ++idx)
    {
      //Synthesize base and detail layers
      itk::ImageRegionConstIterator<TOutputImage> inputIterator(m_LevelBaseImages[idx], region);
      itk::ImageRegionConstIterator<TOutputImage> detailIterator(m_LevelDetailImages[idx], region);
      itk::ImageRegionIterator<TOutputImage> outputIterator(base, region);
      itk::ImageRegionIterator<TOutputImage> outputDetailIterator(detail, region);
      while(!inputIterator.IsAtEnd())
      {
        //outputIterator.Set(m_BaseWeights[idx]*inputIterator.Get()+outputIterator.Get());
        outputIterator.Set(inputIterator.Get()*inputIterator.Get() + outputIterator.Get()); //sums of squares
        outputDetailIterator.Set(detailIterator.Get() + outputDetailIterator.Get());
        //outputDetailIterator.Set(detailIterator.Get()*detailIterator.Get() + outputDetailIterator.Get()); //sums of squares
        ++inputIterator;
        ++detailIterator;
        ++outputIterator;
        ++outputDetailIterator;
      }

      this->InvokeEvent( ProgressEvent() );
    }
    itk::ImageRegionIterator<TOutputImage> outputIteratorFinal(base, region);
    //itk::ImageRegionIterator<TOutputImage> outputDetailIteratorFinal(detail, region);
    while(!outputIteratorFinal.IsAtEnd())
    {
      outputIteratorFinal.Set(sqrt(outputIteratorFinal.Get())); //sqrt
      //outputDetailIteratorFinal.Set(sqrt(outputDetailIteratorFinal.Get())); //sqrt
      ++outputIteratorFinal;
      //++outputDetailIteratorFinal;
    }

    m_BaseImage = base;
    m_DetailImage = detail;
    std::cout << "Done" << std::endl;

    //Create HDR image
    std::cout << "Creating HDR image ... " << std::endl;
    typename TOutputImage::Pointer output = this->GetOutput();
    output->SetRegions(region);
    output->Allocate();

    itk::ImageRegionConstIterator<TOutputImage> inputIterator(m_BaseImage, region);
    itk::ImageRegionConstIterator<TOutputImage> detailIterator(m_DetailImage, region);
    itk::ImageRegionIterator<TOutputImage> outputIterator(this->GetOutput(), region);
    while(!inputIterator.IsAtEnd())
    {
      outputIterator.Set(inputIterator.Get() + m_Beta*detailIterator.Get());
      ++inputIterator;
      ++detailIterator;
      ++outputIterator;
    }
    std::cout << "Done" << std::endl;

    if(m_SumsOfSquares)
    {
      std::cout << "Sums of Squares Image ... " << std::endl;
      typename TOutputImage::Pointer sosImage = milx::Image<TOutputImage>::BlankImage(0.0, region.GetSize());
      m_SoSImage = milx::Image<TOutputImage>::MatchInformation(sosImage, imageFirst); //ensure images in same space
      for(IndexValueType idx = 0; idx < this->GetNumberOfInputs(); ++idx)
      {
        typename InputImageType::Pointer image = const_cast<InputImageType *>(this->GetInput(idx));

        //Synthesize image
        itk::ImageRegionConstIterator<TOutputImage> imageIterator(image, region);
        itk::ImageRegionIterator<TOutputImage> sosIterator(m_SoSImage, region);
        while(!imageIterator.IsAtEnd())
        {
          sosIterator.Set(imageIterator.Get()*imageIterator.Get() + sosIterator.Get()); //sums of squares
          ++imageIterator;
          ++sosIterator;
        }
      }
      itk::ImageRegionIterator<TOutputImage> sosIteratorFinal(m_SoSImage, region);
      while(!sosIteratorFinal.IsAtEnd())
      {
        sosIteratorFinal.Set(sqrt(sosIteratorFinal.Get())); //sqrt
        ++sosIteratorFinal;
      }
      std::cout << "Done" << std::endl;
    }
    if(m_Average)
    {
      std::cout << "Average Image ... " << std::endl;
      typename TOutputImage::Pointer aveImage = milx::Image<TOutputImage>::BlankImage(0.0, region.GetSize());
      m_AverageImage = milx::Image<TOutputImage>::MatchInformation(aveImage, imageFirst); //ensure images in same space
      for(IndexValueType idx = 0; idx < this->GetNumberOfInputs(); ++idx)
      {
        typename InputImageType::Pointer image = const_cast<InputImageType *>(this->GetInput(idx));

        //Synthesize image
        itk::ImageRegionConstIterator<TOutputImage> imageIterator(image, region);
        itk::ImageRegionIterator<TOutputImage> aveIterator(m_AverageImage, region);
        while(!imageIterator.IsAtEnd())
        {
          aveIterator.Set(imageIterator.Get() / this->GetNumberOfInputs() + aveIterator.Get()); //average
          ++imageIterator;
          ++aveIterator;
        }
      }
      std::cout << "Done" << std::endl;
    }
    if(m_BiasField)
    {
      std::cout << "Bias Field Image ... " << std::endl;
      typename TOutputImage::Pointer biasImage = milx::Image<TOutputImage>::BlankImage(0.0, region.GetSize());
      m_BiasFieldImage = milx::Image<TOutputImage>::MatchInformation(biasImage, imageFirst); //ensure images in same space
      for(IndexValueType idx = 0; idx < this->GetNumberOfInputs(); ++idx)
      {
        typename InputImageType::Pointer image = const_cast<InputImageType *>(this->GetInput(idx));

        //Synthesize image
        itk::ImageRegionConstIterator<TOutputImage> imageIterator(image, region);
        itk::ImageRegionConstIterator<TOutputImage> hdrIterator(this->GetOutput(), region);
        itk::ImageRegionIterator<TOutputImage> biasFieldIterator(m_BiasFieldImage, region);
        while(!imageIterator.IsAtEnd())
        {
          biasFieldIterator.Set(biasFieldIterator.Get() + imageIterator.Get() / hdrIterator.Get()); //scale
          ++imageIterator;
          ++hdrIterator;
          ++biasFieldIterator;
        }
      }
      std::cout << "Done" << std::endl;
    }
  }
  else //tone map
  {
    std::cout << "Tone Mapping ... " << std::endl;
    for(IndexValueType idx = 0; idx < this->GetNumberOfInputs(); ++idx)
    {
      typename InputImageType::Pointer image = const_cast<InputImageType *>(this->GetInput(idx));

      if(!image)
        std::cout << "Warning: Image from Input is NULL" << std::endl;

      typename InputImageType::RegionType region = image->GetLargestPossibleRegion();
      std::cout << "Size of image " << idx << ": " << region.GetSize()[0] << ", " << region.GetSize()[1] << ", " << region.GetSize()[2] << std::endl;

      ComputeToneMapEnhancement(image, m_SigmaRange, m_SigmaDomain, m_Contrast);
    }
  }
}

template< typename TInputImage, typename TOutputImage >
void
HighDynamicRangeImageFilter< TInputImage, TOutputImage >
::CreateMultiLightImageCollection(itk::SmartPointer<TInputImage> image, float range, float domain, int levels)
{
  m_LevelResults.clear();
  m_DiffResults.clear();

  //run MLIC
  itk::SmartPointer<TOutputImage> prevResult = NULL;
  for(size_t level = 0; level < levels; level ++)
    {
      size_t factor = 1 << level;
      std::cout << "\tProcess Level " << level << " with Factor " << factor << std::endl;
      //result = None

      //spatial factor as per Fattal et al. 2007, sec. 4.1
      size_t spatialFactor = 1;
      if(level > 1)
        spatialFactor = 1 << (level-1);
      else if(level == 1)
        spatialFactor = sqrt(3);

      itk::SmartPointer<TOutputImage> currentImage = prevResult;
      if(level == 0)
          currentImage = image;

      // create the filter
      typedef itk::FastBilateralImageFilter<TInputImage, TOutputImage> FilterType;
      typename FilterType::Pointer filter1 = FilterType::New();
        filter1->SetInput(currentImage);
        filter1->SetRangeSigma(range/factor);
        filter1->SetDomainSigma(spatialFactor*domain);
        filter1->SetNumberOfThreads(this->GetNumberOfThreads());
      try
        {
          std::cout << "Applying Level " << level << " ..." << std::endl;
          filter1->Update();
        }
      catch (itk::ExceptionObject& e)
        {
          std::cerr << "Exception detected: "  << e.GetDescription();
          return;
        }
      itk::SmartPointer<TOutputImage> result = filter1->GetOutput();

      //Diff with previous
      typedef itk::SubtractImageFilter<TOutputImage, TOutputImage> SubtractImageType;
      typename SubtractImageType::Pointer subtractFilter = SubtractImageType::New();
        subtractFilter->SetInput1(currentImage);
        subtractFilter->SetInput2(result);
        try
        {
          subtractFilter->Update();
        }
        catch (itk::ExceptionObject & ex )
        {
          std::cerr << "Failed Subtraction" << std::endl;
          std::cerr << ex.GetDescription() << std::endl;
        }
      itk::SmartPointer<TOutputImage> diffResult = subtractFilter->GetOutput();

      m_LevelResults.push_back(result);
      m_DiffResults.push_back(diffResult);
      prevResult = result;
    }
}

template< typename TInputImage, typename TOutputImage >
void
HighDynamicRangeImageFilter< TInputImage, TOutputImage >
::ComputeMultiscaleShapeDetailEnhancement(std::vector< itk::SmartPointer<TOutputImage> > results, std::vector< itk::SmartPointer<TOutputImage> > diffs, RegionType region, int levels, float lambdaValue)
{
    if(results.empty() || diffs.empty())
    {
        std::cout << "Inputs to MSDE cannot be empty. Returning.";
        return;
    }

    float epsilon = 1e-8; //avoid divide by zero
    float lambdaValues[3] = { lambdaValue, lambdaValue + 0.05, lambdaValue + 0.15 };
    typename InputImageType::Pointer levelDetailBlank = milx::Image<TOutputImage>::BlankImage(0.0, region.GetSize());
    m_DetailImage = milx::Image<TOutputImage>::MatchInformation(levelDetailBlank, results[0]); //ensure images in same space
    for(size_t level = 0; level < levels; level ++)
      {
        float lambda = lambdaValue;
        if (levels == 3) //3 level then use equalizer
          lambda = lambdaValues[level];

        std::cout << "\tProcessing image in level " << level << " with lambda of " << lambda << std::endl;
        typename TOutputImage::Pointer weightsBlank = milx::Image<TOutputImage>::BlankImage(0.0, region.GetSize());
        typename TOutputImage::Pointer weights = milx::Image<TOutputImage>::MatchInformation(weightsBlank, results[0]); //ensure images in same space

        typename TOutputImage::Pointer gradMagResult = milx::Image<TOutputImage>::GradientMagnitude(results[level]);

        typedef itk::MinimumImageFunction<TOutputImage> FilterType;
        typename FilterType::Pointer minImageFunction = FilterType::New();
        minImageFunction->SetInputImage(results[level]);

        //std::cout << "Computing Weights ... " << std::endl;
        itk::ImageRegionIteratorWithIndex<TOutputImage> resultIterator(results[level], region);
        itk::ImageRegionIteratorWithIndex<TOutputImage> diffIterator(diffs[level], region);
        itk::ImageRegionIteratorWithIndex<TOutputImage> gradIterator(gradMagResult, region);
        itk::ImageRegionIteratorWithIndex<TOutputImage> weightsIterator(weights, region);
        while(!diffIterator.IsAtEnd())
            {
              PixelType minValue = static_cast<PixelType>(minImageFunction->EvaluateAtIndex(resultIterator.GetIndex()));
              PixelType C = gradIterator.Get()/(minValue+epsilon); //penalise strong edges which the ideal bilateral would not have picked up
              // Set the current detail pixel
              PixelType U = exp( abs(diffIterator.Get())-C );
              weightsIterator.Set(U);

              //reduce ratio between min max values
              diffIterator.Set(copysign(pow(fabs(diffIterator.Get()), lambda), diffIterator.Get())); //copysign - Return x with the sign of y

              ++resultIterator;
              ++diffIterator;
              ++gradIterator;
              ++weightsIterator;
            }
        //std::cout << "Done" << std::endl;

        //Smooth weights
        typename TOutputImage::Pointer weightsFinal = milx::Image<TOutputImage>::GaussianSmooth(weights, 1); //smooth, 8 parameter from Fattal et al. 2007

        //Muliply, Add and Deep Copy detail
        //std::cout << "Form level layers ... " << std::endl;
        itk::ImageRegionConstIterator<TOutputImage> inputIterator(diffs[level], region);
        itk::ImageRegionConstIterator<TOutputImage> weightIterator(weightsFinal, region);
        itk::ImageRegionIterator<TOutputImage> outputIterator(m_DetailImage, region);
        while(!inputIterator.IsAtEnd())
          {
              outputIterator.Set(inputIterator.Get()*weightIterator.Get()+outputIterator.Get());
              ++inputIterator;
              ++weightIterator;
              ++outputIterator;
          }

        if (level == results.size() - 1) //last one
          m_BaseImage = results[level];
        //std::cout << "Done" << std::endl;
      }
}

template< typename TInputImage, typename TOutputImage >
void
HighDynamicRangeImageFilter< TInputImage, TOutputImage >
::ComputeToneMapEnhancement(itk::SmartPointer<TInputImage> image, float range, float domain, float contrast)
{
  typename InputImageType::RegionType region = image->GetLargestPossibleRegion();

  //log of image
  typedef itk::LogImageFilter<TInputImage, TOutputImage> LogFilterType;
  typename LogFilterType::Pointer filter = LogFilterType::New();
  filter->SetInput(image);
  filter->SetNumberOfThreads(this->GetNumberOfThreads());
  try
  {
    std::cout << "Applying Log function" << std::endl;
    filter->Update();
  }
  catch(itk::ExceptionObject& e)
  {
    std::cerr << "Exception detected: " << e.GetDescription();
    return;
  }
  itk::SmartPointer<TOutputImage> logImage = filter->GetOutput();

  // bilateral filter
  typedef itk::FastBilateralImageFilter<TOutputImage, TOutputImage> FilterType;
  typename FilterType::Pointer filter1 = FilterType::New();
  filter1->SetInput(logImage);
  filter1->SetRangeSigma(range);
  filter1->SetDomainSigma(domain);
  filter1->SetNumberOfThreads(this->GetNumberOfThreads());
  try
  {
    std::cout << "Applying Bilateral Filter" << std::endl;
    filter1->Update();
  }
  catch(itk::ExceptionObject& e)
  {
    std::cerr << "Exception detected: " << e.GetDescription();
    return;
  }
  m_BaseImage = filter1->GetOutput();

  //min/max for scaling
  typedef itk::MinimumMaximumImageCalculator<TOutputImage> ImageCalculatorFilterType;
  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New();
  imageCalculatorFilter->SetImage(m_BaseImage);
  imageCalculatorFilter->Compute();
  double maxValue = imageCalculatorFilter->GetMaximum();
  double minValue = imageCalculatorFilter->GetMinimum();
  std::cout << "Min/Max in log image: " << minValue << "/" << maxValue << std::endl;

  double scale = contrast / static_cast<double>(maxValue - minValue);
  const double normFactor = 1.0/exp(maxValue*scale);
  std::cout << "Applying Scaling in Log Domain of " << scale << std::endl;

  //Apply scaling in log domain
  typename TOutputImage::Pointer scaledImage = milx::Image<TOutputImage>::BlankImage(0.0, region.GetSize());
  m_DetailImage = milx::Image<TOutputImage>::MatchInformation(scaledImage, image); //ensure images in same space
  itk::ImageRegionConstIterator<TOutputImage> logIterator(logImage, region);
  itk::ImageRegionConstIterator<TOutputImage> baseIterator(m_BaseImage, region);
  itk::ImageRegionIterator<TOutputImage> detailIterator(m_DetailImage, region);
  itk::ImageRegionIterator<TOutputImage> outputIterator(this->GetOutput(), region);
  while(!logIterator.IsAtEnd())
  { 
    //Create detail layer
    double detailValue = logIterator.Get() - baseIterator.Get(); //division in real space
    detailIterator.Set(detailValue);
    //compose HDR tone mapped image
    double expValue = exp(baseIterator.Get()*scale + detailValue); //multiply in real space
    //rescale to 0-1 range
    double toneMapValue = expValue*normFactor;
    outputIterator.Set(toneMapValue);

    ++logIterator;
    ++baseIterator;
    ++detailIterator;
    ++outputIterator;
  }
}

} // end namespace itk

#endif
