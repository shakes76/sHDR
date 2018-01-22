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
#ifndef itkHighDynamicRangeImageFilter_h
#define itkHighDynamicRangeImageFilter_h

#include <itkImageToImageFilter.h>

namespace itk
{
//HDR Mode
enum HDRMode { ToneMap = 0, MultiLight };

/** \class HighDynamicRangeImageFilter
 * \brief Combine N images into an HDR image using various HDR techniques
 *
 * This filter combines the N images provided into a single HDR image.
 * Types of HDR supported include tone mapping and multi-light image collections.
 *
 * \author Shekhar S. Chandra
 *
 * \ingroup ITKImageCompose
 */
template< typename TInputImage, typename TOutputImage >
class HighDynamicRangeImageFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef HighDynamicRangeImageFilter                           Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(HighDynamicRangeImageFilter, ImageToImageFilter);

  /** Compiler can't inherit typedef? */
  typedef typename Superclass::DataObjectPointerArraySizeType  DataObjectPointerArraySizeType;
  typedef typename Superclass::InputImageType  InputImageType;
  typedef typename Superclass::OutputImageType OutputImageType;
  typedef typename InputImageType::PixelType   PixelType;
  typedef typename InputImageType::RegionType  RegionType;
  typedef typename InputImageType::Pointer     InputImagePointer;
  typedef typename OutputImageType::Pointer    OutputImagePointer;
  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  /** Sets first NULL indexed input, appends to the end otherwise */
  virtual inline void AddInput(const InputImageType *input)
  {
    // Forward to the protected method in the superclass
    Superclass::AddInput(const_cast<InputImageType*>(input));
  }
  /** Define the number of indexed inputs defined for this
  * process. The new indexed inputs are considered to be NULL. If the
  * size is a reduction then those elements are removed.
  */
  void SetNumberOfIndexedInputs(DataObjectPointerArraySizeType num)
  {
    Superclass::SetNumberOfIndexedInputs(num);
  }

  /** Set/Get number of levels */
  itkSetMacro(Levels, int);
  itkGetConstMacro(Levels, int);
  /** Set/Get sums of squares image computation */
  itkSetMacro(SumsOfSquares, bool);
  itkGetConstMacro(SumsOfSquares, bool);
  itkBooleanMacro(SumsOfSquares);
  /** Set/Get average image computation */
  itkSetMacro(Average, bool);
  itkGetConstMacro(Average, bool);
  itkBooleanMacro(Average);
  /** Set/Get bias field image computation */
  itkSetMacro(BiasField, bool);
  itkGetConstMacro(BiasField, bool);
  itkBooleanMacro(BiasField);
  /** Set/Get beta value of detail level */
  itkSetMacro(Beta, float);
  itkGetConstMacro(Beta, float);
  /** Set/Get lambda value of detail level */
  itkSetMacro(Lambda, float);
  itkGetConstMacro(Lambda, float);
  /** Set/Get sigma range of feature extraction */
  itkSetMacro(SigmaRange, float);
  itkGetConstMacro(SigmaRange, float);
  /** Set/Get sigma domain of feature extraction */
  itkSetMacro(SigmaDomain, float);
  itkGetConstMacro(SigmaDomain, float);
  /** Set/Get contrast of tone map enhancement */
  itkSetMacro(Contrast, float);
  itkGetConstMacro(Contrast, float);

  void ToneModeModeOn()
  {   m_Mode = ToneMap;   }  
  void MultiLightModeOn()
  {   m_Mode = MultiLight;   }  

  /** Get the MLIC result*/
  itk::SmartPointer<OutputImageType> GetBaseImage()
  {
    return m_BaseImage;
  }
  /** Get the MLIC result*/
  itk::SmartPointer<OutputImageType> GetDetailImage()
  {
      return m_DetailImage;
  }
  /** Get the sums of squares result*/
  itk::SmartPointer<OutputImageType> GetSumsOfSquaresImage()
  {
    return m_SoSImage;
  }
  /** Get the average result*/
  itk::SmartPointer<OutputImageType> GetAverageImage()
  {
    return m_AverageImage;
  }
  /** Get the channel usage result*/
  itk::SmartPointer<OutputImageType> GetChannelImage()
  {
    return m_ChannelImage;
  }
  /** Get the bias field result*/
  itk::SmartPointer<OutputImageType> GetBiasFieldImage()
  {
    return m_BiasFieldImage;
  }

  /** Set Individual weights for the base images */
  inline void AddInputWeight(float weight)
  {
    m_BaseWeights.push_back(weight);
  }
  /** Get Individual weights for the base images */
  inline float GetInputWeight(int idx)
  {
    return m_BaseWeights[idx];
  }
  /** Get Individual weights for the base images */
  inline std::vector< float > GetInputWeights()
  {
    return m_BaseWeights;
  }
  /** Individual weights for the base images */
  inline void ClearInputWeights()
  {
    m_BaseWeights.clear();
  }

  /** Get the MSDE base result for a given level*/
  inline itk::SmartPointer<OutputImageType> GetLevelBaseImage(int level)
  {
    return m_LevelBaseImages[level];
  }
  /** Get the MSDE detail result for a given level*/
  inline itk::SmartPointer<OutputImageType> GetLevelDetailImage(int level)
  {
    return m_LevelDetailImages[level];
  }

  /** Get the MLIC base result*/
  inline std::vector< itk::SmartPointer<OutputImageType> > GetMultiLightResults()
  {
    return m_LevelResults;
  }
  /** Get the MLIC detail details*/
  inline std::vector< itk::SmartPointer<OutputImageType> > GetMultiLightDetails()
  {
    return m_DiffResults;
  }

  /**Create Multi-light Image Collection (MLIC) using the fast bilateral filter.
     Issues on Windows: optimisation flag /O2 and maybe the flag /Ob2 causes crash. Using /O1 and /Ob1 worked OK.*/
  void CreateMultiLightImageCollection(itk::SmartPointer<TInputImage> image, float range, float domain, int levels);
  void ComputeMultiscaleShapeDetailEnhancement(std::vector< itk::SmartPointer<TOutputImage> > results, std::vector< itk::SmartPointer<TOutputImage> > diffs, RegionType region, int levels, float lambdaValue = 0.8);
  //Tone mapping of Durand et al.
  void ComputeToneMapEnhancement(itk::SmartPointer<TInputImage> image, float range, float domain, float contrast = 5);

protected:
  HighDynamicRangeImageFilter();
  ~HighDynamicRangeImageFilter() {}

  void PrintSelf(std::ostream & os, Indent indent) const ITK_OVERRIDE;

  /** Override VeriyInputInformation() to add the additional check
   * that all inputs have the same number of components.
   *
   * \sa ProcessObject::VerifyInputInformation
   */
//  virtual void VerifyInputInformation() ITK_OVERRIDE;

  /** Overrides GenerateInputRequestedRegion() in order to inform
   * the pipeline execution model of different input requested regions
   * than the output requested region.
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
//  virtual void GenerateInputRequestedRegion() ITK_OVERRIDE;

  /** HighDynamicRangeImageFilter can be implemented as a multithreaded filter.
   * \sa ImageSource::ThreadedGenerateData(),
   *     ImageSource::GenerateData() */
//  virtual void ThreadedGenerateData(const OutputImageRegionType &
//                                    outputRegionForThread, ThreadIdType threadId) ITK_OVERRIDE;
  virtual void GenerateData() ITK_OVERRIDE;

  int m_Levels; //!< Number of levels in algorithm
  bool m_SumsOfSquares; //!< Compute SoS Image?
  bool m_Average; //!< Compute Average Image?
  bool m_BiasField; //!< Compute Bias Field Image?
  float m_Beta; //!< Detail fusion parameter for HDR MSDE
  float m_Lambda; //!< Detail enhancement parameter for HDR MSDE
  float m_SigmaRange; //!< Range parameter of Features
  float m_SigmaDomain; //!< Domain parameter of Features
  float m_Contrast; //!< Contrast enhancement
  HDRMode m_Mode; //!< HDR Mode to use

  itk::SmartPointer<OutputImageType> m_BaseImage;
  itk::SmartPointer<OutputImageType> m_DetailImage;
  itk::SmartPointer<OutputImageType> m_SoSImage;
  itk::SmartPointer<OutputImageType> m_AverageImage;
  itk::SmartPointer<OutputImageType> m_ChannelImage;
  itk::SmartPointer<OutputImageType> m_BiasFieldImage;
  std::vector< float > m_BaseWeights;
  std::vector< itk::SmartPointer<OutputImageType> > m_LevelBaseImages;
  std::vector< itk::SmartPointer<OutputImageType> > m_LevelDetailImages;
  std::vector< itk::SmartPointer<OutputImageType> > m_LevelResults;
  std::vector< itk::SmartPointer<OutputImageType> > m_DiffResults;

private:
  HighDynamicRangeImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);        //purposely not implemented

};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHighDynamicRangeImageFilter.hxx"
#endif

#endif
