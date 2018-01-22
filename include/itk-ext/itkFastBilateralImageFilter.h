#ifndef __itkFastBilateralImageFilter_h
#define __itkFastBilateralImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkLinearInterpolateImageFunction.h"

namespace itk
{
/**
* \class FastBilateralImageFilter
* \brief A fast approximation to the bilateral filter
* 
* This filter is a fast approximation to the bilateral filter.
* Blurring is performed on an image based on the distance of pixels in
* both space and intensity.
*
* The algorithm used was originally proposed by Paris and
* Durand [1].
*
* Instead of calculating a kernel for every pixel in
* an image, this filter places the values of each pixel into a higher
* dimensional image determined by the position and intensity of a pixel.
* How many bins are used is determined by the sigma values provided
* to the filter. Larger sigmas will result in more aggresive downsampling
* and less running time overall. After the data of an image
* has been organized into bins, a DiscreteGaussianImageFilter is applied.
* Finally, the output image is constructed by interpolating the
* values of the output pixels from the blurred higher
* dimensional image.
* 
* [1] Sylvain Paris and Frédo Durand,
*     A Fast Approximation of the Bilateral Filter using a Signal Processing
*     Approach,
*     European Conference on Computer Vision (ECCV'06)
* 
* \sa BilateralImageFilter
* \sa GaussianOperator
* \sa AnisotropicDiffusionImageFilter
* \sa Image
* \sa Neighborhood
* \sa NeighborhoodOperator
*
* \ingroup ImageEnhancement
* \ingroup ImageFeatureExtraction
*
* \todo Support for color images
* \todo Support for vector images
*/

template <class TInputImage, class TOutputImage >
class ITK_EXPORT FastBilateralImageFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  
  /** Standard class typedefs. */
  typedef FastBilateralImageFilter                          Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage >   Superclass;
  typedef SmartPointer<Self>                                Pointer;
  typedef SmartPointer<const Self>                          ConstPointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(FastBilateralImageFilter, ImageToImageFilter);
  
  /** Dimensionality of the input image. Dimensionality of the output image
   *  is assumed to be the same. */
  itkStaticConstMacro(
    ImageDimension, unsigned int, TInputImage::ImageDimension);
  
  /** Input image typedefs. */
  typedef TInputImage                                   InputImageType;
  typedef typename TInputImage::Pointer                 InputImagePointer;
  typedef typename TInputImage::ConstPointer            InputImageConstPointer;
  typedef typename TInputImage::SpacingType             InputImageSpacingType;
  typedef typename TInputImage::SizeType                InputImageSizeType;
  typedef typename TInputImage::IndexType               InputImageIndexType;
  
  /** Input image iterator type. */
  typedef ImageRegionConstIteratorWithIndex<TInputImage>
    InputImageConstIteratorType;
  
  /** Output image typedefs. */
  typedef TOutputImage                                  OutputImageType;
  typedef typename TOutputImage::Pointer                OutputImagePointer;
  
  /** Output image iterator type. */
  typedef ImageRegionIterator<TOutputImage>
    OutputImageIteratorType;
  
  /** Pixel types. */
  typedef typename TOutputImage::PixelType              OutputPixelType;
  typedef typename TInputImage::PixelType               InputPixelType;
  
  /** Typedef for an array of doubles that specifies the DomainSigma
   *  in each spacial dimension. */
  typedef FixedArray<double, itkGetStaticConstMacro(ImageDimension)>
    DomainSigmaArrayType;
  
  /** Standard get/set macros for filter parameters.
   *  DomainSigma is specified in the same units as the Image spacing.
   *  RangeSigma is specified in the units of intensity. */
  itkGetConstMacro(DomainSigma, const DomainSigmaArrayType);
  itkSetMacro(DomainSigma, DomainSigmaArrayType);
  itkGetConstMacro(RangeSigma, double);
  itkSetMacro(RangeSigma, double);
  
  /** Convenience set method for setting all domain standard deviations to the
   *  same value. */
  void SetDomainSigma(const double v)
    {
    m_DomainSigma.Fill(v);
    }
  
protected:
  
  /** Default Constructor. Default value for DomainSigma is 4. Default
   *  value for RangeSigma is 50. These values were chosen match those of the
   *  BilateralImageFilter */
  FastBilateralImageFilter()
    {
    m_DomainSigma.Fill(4.0);
    m_RangeSigma = 50.0;
    }
  
  virtual ~FastBilateralImageFilter() {}
  
  /*
   * The FastBilateralImageFilter needs a larger input requested
   * region than the size of the output requested region. Like
   * the BilateralImageFilter, the FastBilateralImageFilter needs
   * an amount of padding in each dimension based on the domain sigma.
   */
  virtual void GenerateInputRequestedRegion()
    throw(InvalidRequestedRegionError);

  /** Standard pipline method */
  void GenerateData();
  
  /** Method to print member variables to an output stream */
  void PrintSelf(std::ostream& os, Indent indent) const;
  
  /** The type of image to use as the higher dimensional grid.
   * The blurring is performed on this image type. */
  typedef typename
  itk::Image<float, itkGetStaticConstMacro(ImageDimension)+1>
    GridType;
  
  /** Grid types */
  typedef typename GridType::PixelType                  GridPixelType;
  typedef typename GridType::IndexType                  GridIndexType;
  typedef typename GridType::SizeType                   GridSizeType;
  typedef typename
    Size<itkGetStaticConstMacro(ImageDimension)+1>::SizeValueType
                                                        GridSizeValueType;
  typedef typename GridType::RegionType                 GridRegionType;
  
  /** Grid image iterator type. */
  typedef ImageRegionIterator<GridType>                 GridImageIteratorType;
  typedef ImageRegionConstIterator<GridType>
    GridImageConstIteratorType;

  /** The type of blurring to use on the grid. */
  typedef DiscreteGaussianImageFilter< GridType, GridType > BlurType;
 
  /** The type of interpolation done to calculate output pixels. */ 
  typedef LinearInterpolateImageFunction<GridType, float>
    InterpolatorType;
  typedef typename InterpolatorType::ContinuousIndexType
    InterpolatedIndexType;
  
private:
  
  FastBilateralImageFilter(const Self&);  // Not implemented on purpose
  
  void operator=(const Self&);            // Not implemented on purpose
  
  double                m_RangeSigma;
  DomainSigmaArrayType  m_DomainSigma;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFastBilateralImageFilter.txx"
#endif

#endif // End #ifndef __itkFastBilateralImageFilter_h

