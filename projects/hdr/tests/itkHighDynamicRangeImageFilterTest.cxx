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

#include "itkHighDynamicRangeImageFilter.h"
#include "itkStreamingImageFilter.h"

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

  if(argc < 7)
    {
    std::cerr << "Usage: " << av[0] << " InputImage1 InputImage2 InputImage3 sigmaRange sigmaDomain OutputImage\n";
    return -1;
    }

  float range = atof(av[4]);
  float domain = atof(av[5]);

  // load images
  itk::ImageFileReader<InputImageType>::Pointer input1 = itk::ImageFileReader<InputImageType>::New();
  input1->SetFileName(argv[1]);
  itk::ImageFileReader<InputImageType>::Pointer input2 = itk::ImageFileReader<InputImageType>::New();
  input2->SetFileName(argv[2]);
  itk::ImageFileReader<InputImageType>::Pointer input3 = itk::ImageFileReader<InputImageType>::New();
  input3->SetFileName(argv[3]);

  try
    {
    input1->Update();
    input2->Update();
    input3->Update();
    std::cout << "Using range and domain sigma as: " << range << ", " << domain << std::endl;
    myImage::RegionType region = input->GetOutput()->GetLargestPossibleRegion();
    std::cout << "Size of image read: " << region.GetSize()[0] << ", " << region.GetSize()[1] << ", " << region.GetSize()[2] << std::endl;
    }
  catch (itk::ExceptionObject& e)
    {
    std::cerr << "Exception detected: "  << e.GetDescription();
    return -1;
    }

  // create the filter
  typedef itk::HighDynamicRangeImageFilter< InputImageType, OutputImageType >
    HighDynamicRangeImageType;
  HighDynamicRangeImageType::Pointer HDRImage = HighDynamicRangeImageType::New();

  // check the default values
  if ( HDRImage->GetSpacing() != 1.0 )
    {
    std::cout << "Default spacing is not 1.0" << std::endl;
    return EXIT_FAILURE;
    }
  if ( HDRImage->GetOrigin() != 0.0 )
    {
    std::cout << "Default origin is not 0.0" << std::endl;
    return EXIT_FAILURE;
    }

  // setup the filter
  HDRImage->SetSpacing( spacingValue );
  HDRImage->SetOrigin( originValue );
  for ( int i = 0; i < numInputs; i++ )
    {
    HDRImage->SetInput( i, inputs[i] );
    }

  // to test ProgressReporter
  ShowProgressObject progressWatch( HDRImage );
  typedef itk::SimpleMemberCommand< ShowProgressObject > CommandType;
  CommandType::Pointer command = CommandType::New();
  command->SetCallbackFunction( &progressWatch,
                                &ShowProgressObject::ShowProgress );
  HDRImage->AddObserver( itk::ProgressEvent(), command );

  // to test streaming
  typedef itk::StreamingImageFilter< OutputImageType, OutputImageType >
    StreamingImageType;
  StreamingImageType::Pointer streamingImage = StreamingImageType::New();
  streamingImage->SetInput( HDRImage->GetOutput() );
  streamingImage->SetNumberOfStreamDivisions( streamDivisions );


  // run
  try
    {
    streamingImage->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cout << err << std::endl;
    //for InvalidRequestedRegionError
    itk::DataObjectError * errp = dynamic_cast<itk::DataObjectError *>( &err );
    if ( errp )
      {
      errp->GetDataObject()->Print( std::cout );
      }
    return EXIT_FAILURE;
    }

  OutputImageType::Pointer output = streamingImage->GetOutput();


  // check the informations
  if ( output->GetLargestPossibleRegion() != expectedRegion )
    {
    std::cout << "LargestPossibleRegion mismatch" << std::endl;
    return EXIT_FAILURE;
    }
  if ( output->GetSpacing() != expectedSpacing )
    {
    std::cout << "Spacing mismatch" << std::endl;
    return EXIT_FAILURE;
    }
  if ( output->GetOrigin() != expectedOrigin )
    {
    std::cout << "Origin mismatch" << std::endl;
    return EXIT_FAILURE;
    }

  // check the contents
  bool passed = true;

  PixelType counter2 = 0;
  itk::ImageRegionIterator<OutputImageType>
    outputIter( output, output->GetBufferedRegion() );
  while ( !outputIter.IsAtEnd() )
    {
    if ( outputIter.Get() != counter2 )
      {
      passed = false;
      std::cout << "Mismatch at index: " << outputIter.GetIndex() << std::endl;
      }
    ++counter2;
    ++outputIter;
    }

  if ( !passed || counter1 != counter2 )
    {
    std::cout << "Test failed." << std::endl;
    return EXIT_FAILURE;
    }


  // An exception is raised when an input is missing.
  passed = false;

  // set the 2nd input null
  HDRImage->SetInput( 1, ITK_NULLPTR );
  try
    {
    HDRImage->Update();
    }
  catch( itk::InvalidRequestedRegionError & err )
    {
    std::cout << err << std::endl;
    passed = true;
    }
  catch( itk::ExceptionObject & err )
    {
    std::cout << err << std::endl;
    return EXIT_FAILURE;
    }

  if ( !passed )
    {
    std::cout << "Expected exception is missing" << std::endl;
    return EXIT_FAILURE;
    }


  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}
