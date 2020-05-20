
#pragma once

#include <itkConfigure.h>
#if(ITK_VERSION_MAJOR == 4) 
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-field"
#endif
#endif

#include "itkImage.h"

#define ITK_IMAGE_READER_HELPER_ImageDataArrayName()                                                                                                                                                   \
private:                                                                                                                                                                                               \
  QString m_FileName;                                                                                                                                                                                  \
  QString getFileName()                                                                                                                                                                                \
  {                                                                                                                                                                                                    \
    return m_FileName;                                                                                                                                                                                 \
  }                                                                                                                                                                                                    \
  void setFileName(const QString& value)                                                                                                                                                               \
  {                                                                                                                                                                                                    \
    m_FileName = value;                                                                                                                                                                                \
  }

#define ITK_IMAGE_READER_HELPER_DECL()                                                                                                                                                                 \
  template <typename TComponent>                                                                                                                                                                       \
  void readImage(const DataArrayPath& dataArrayPath, const itk::ImageIOBase::Pointer& imageIO, const QString& filename, bool dataCheck);                                                               \
                                                                                                                                                                                                       \
  template <typename TComponent, unsigned int dimensions>                                                                                                                                              \
  void readImage(const DataArrayPath& dataArrayPath, const itk::ImageIOBase::Pointer& imageIO, const QString& filename, bool dataCheck);                                                               \
                                                                                                                                                                                                       \
  template <typename TPixel, unsigned int>                                                                                                                                                             \
  void readImage(const DataArrayPath& dataArrayPath, const QString& filename, bool dataCheck);                                                                                                         \
                                                                                                                                                                                                       \
  void readImage(const DataArrayPath& dataArrayPath, bool dataCheck);                                                                                                                                  \
                                                                                                                                                                                                       \
  template <typename TPixel, unsigned int dimensions>                                                                                                                                                  \
  void readImageOutputInformation(const DataArrayPath& dataArrayPath, typename itk::ImageFileReader<itk::Image<TPixel, dimensions>>::Pointer& reader, DataContainer::Pointer& container);



#ifdef __clang__
#pragma clang diagnostic pop
#endif
