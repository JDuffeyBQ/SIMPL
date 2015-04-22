/* ============================================================================
 * Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "PipelineViewWidget.h"

#include <iostream>


#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>
#include <QtCore/QMimeData>

#include <QtGui/QMouseEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragLeaveEvent>
#include <QtGui/QDragMoveEvent>
#include <QtWidgets/QLabel>
#include <QtGui/QPixmap>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QMessageBox>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/Common/PipelineMessage.h"
#include "DREAM3DLib/Common/FilterManager.h"
#include "DREAM3DLib/Common/IFilterFactory.hpp"
#include "DREAM3DLib/Common/FilterFactory.hpp"
#include "DREAM3DLib/FilterParameters/QFilterParametersReader.h"
#include "DREAM3DLib/FilterParameters/QFilterParametersWriter.h"
#include "DREAM3DLib/FilterParameters/JsonFilterParametersReader.h"
#include "DREAM3DLib/FilterParameters/JsonFilterParametersWriter.h"

#include "QtSupport/QDroppableScrollArea.h"

#include "DREAM3DWidgetsLib/FilterWidgetManager.h"



#include "DREAM3DWidgetsLib/FilterParameterWidgets/FilterParameterWidgetsDialogs.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewWidget::PipelineViewWidget(QWidget* parent) :
  QFrame(parent),
  m_SelectedFilterWidget(NULL),
  m_FilterWidgetLayout(NULL),
  m_FilterBeingDragged(NULL),
  m_DropIndex(-1),
  m_EmptyPipelineLabel(NULL),
  m_ScrollArea(NULL),
  m_AutoScroll(false),
  m_AutoScrollMargin(10),
  m_autoScrollCount(0),
  m_PipelineMessageObserver(NULL)
{
  setupGui();
  m_LastDragPoint = QPoint(-1, -1);
  m_autoScrollTimer.setParent(this);

  setContextMenuPolicy(Qt::CustomContextMenu);
  setFocusPolicy(Qt::StrongFocus);

  connect(this,
          SIGNAL(customContextMenuRequested(const QPoint&)),
          SLOT(on_customContextMenuRequested(const QPoint&)));

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewWidget::~PipelineViewWidget()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setPipelineMessageObserver(QObject* pipelineMessageObserver)
{
  m_PipelineMessageObserver = pipelineMessageObserver;
  // setup our connection
  connect(this, SIGNAL(pipelineIssuesCleared()),
          m_PipelineMessageObserver, SLOT(clearIssues()) );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setupGui()
{
  newEmptyPipelineViewLayout();
  connect(&m_autoScrollTimer, SIGNAL(timeout()), this, SLOT(doAutoScroll()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setScrollArea(QScrollArea* sa)
{
  m_ScrollArea = sa;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::newEmptyPipelineViewLayout()
{
  if(m_EmptyPipelineLabel == NULL)
  {
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    QSpacerItem* verticalSpacer = new QSpacerItem(20, 341, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addItem(verticalSpacer, 0, 1, 1, 1);

    QSpacerItem* horizontalSpacer_3 = new QSpacerItem(102, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(horizontalSpacer_3, 1, 0, 1, 1);

    m_EmptyPipelineLabel = new QLabel(this);
    m_EmptyPipelineLabel->setObjectName(QString::fromUtf8("label"));
    m_EmptyPipelineLabel->setMinimumSize(QSize(325, 50));
    m_EmptyPipelineLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
                                                          "border-radius: 20px;\n"
                                                          "border: 1px solid rgb(120, 120, 120);\n"
                                                          "background-color: rgb(160, 160, 160);\n"
                                                          "font-weight: bold;\n"
                                                          "color : rgb(255, 255, 255);\n"
                                                          "text-align: center;\n"
                                                          "margin: 5px;\n"
                                                          "padding: 10px;\n"
                                                          "}"));
    m_EmptyPipelineLabel->setAlignment(Qt::AlignCenter);
    m_EmptyPipelineLabel->setText("Drag filters here to build up a pipeline");

    gridLayout->addWidget(m_EmptyPipelineLabel, 1, 1, 1, 1);

    QSpacerItem* horizontalSpacer_4 = new QSpacerItem(102, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(horizontalSpacer_4, 1, 2, 1, 1);

    QSpacerItem* verticalSpacer_2 = new QSpacerItem(20, 341, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addItem(verticalSpacer_2, 2, 1, 1, 1);
  }
  emit pipelineTitleUpdated(QString("Untitled Pipeline"));
  emit pipelineChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineViewWidget::filterCount()
{
  int count = 0;
  if (NULL != m_FilterWidgetLayout)
  {
    count = m_FilterWidgetLayout->count();
  }
  return count;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineFilterWidget* PipelineViewWidget::filterWidgetAt(int index)
{
  PipelineFilterWidget* fw = NULL;
  if (m_FilterWidgetLayout != NULL)
  {
    QWidget* w = m_FilterWidgetLayout->itemAt(index)->widget();
    fw = qobject_cast<PipelineFilterWidget*>(w);
  }
  return fw;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::resetLayout()
{
  // Check to see if we have removed all the filters
  if (filterCount() <= 0)
  {
    // Emit a signal to tell DREAM3D_UI to erase the Filter Input Widget.
    emit noFilterWidgetsInPipeline();

    // Remove the current Layout
    QLayout* l = layout();
    if (NULL != l && l == m_FilterWidgetLayout)
    {
      qDeleteAll(l->children());
      delete l;
      m_FilterWidgetLayout = NULL;
    }

    // and add the empty pipeline layout instead
    newEmptyPipelineViewLayout();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::clearWidgets()
{
  emit pipelineIssuesCleared();
  emit pipelineChanged();

  qint32 count = filterCount();
  for(qint32 i = count - 1; i >= 0; --i)
  {
    QWidget* w = m_FilterWidgetLayout->itemAt(i)->widget();
    QSpacerItem* spacer = m_FilterWidgetLayout->itemAt(i)->spacerItem();
    if (NULL != w)
    {
      m_FilterWidgetLayout->removeWidget(w);
      PipelineFilterWidget* fw = qobject_cast<PipelineFilterWidget*>(w);
      if(fw)
      {
        fw->getFilter()->setPreviousFilter(AbstractFilter::NullPointer());
        fw->getFilter()->setNextFilter(AbstractFilter::NullPointer());
      }
      w->deleteLater();
    }
    else if (NULL != spacer)
    {
      m_FilterWidgetLayout->removeItem(spacer);
    }
  }
  m_SelectedFilterWidget = NULL;
  resetLayout();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::reindexWidgetTitles()
{
  qint32 count = filterCount();
  for(qint32 i = 0; i < count; ++i)
  {
    PipelineFilterWidget* fw = filterWidgetAt(i);
    if (fw)
    {
      QString hl = fw->getFilter()->getHumanLabel();
      hl = QString("[") + QString::number(i + 1) + QString("] ") + hl;
      fw->setFilterTitle(hl);
    }
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer PipelineViewWidget::getFilterPipeline()
{
  // Create a Pipeline Object and fill it with the filters from this View
  FilterPipeline::Pointer pipeline = FilterPipeline::New();

  qint32 count = filterCount();
  for(qint32 i = 0; i < count; ++i)
  {
    PipelineFilterWidget* fw = filterWidgetAt(i);
    if (fw)
    {
      AbstractFilter::Pointer filter = fw->getFilter();
      pipeline->pushBack(filter);
    }

  }
  pipeline->addMessageReceiver(m_PipelineMessageObserver);
  return pipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer PipelineViewWidget::getCopyOfFilterPipeline()
{
  // Create a Pipeline Object and fill it with the filters from this View
  FilterPipeline::Pointer pipeline = FilterPipeline::New();

  qint32 count = filterCount();
  for(qint32 i = 0; i < count; ++i)
  {
    PipelineFilterWidget* fw = filterWidgetAt(i);
    if (fw)
    {
      AbstractFilter::Pointer filter = fw->getFilter();
      AbstractFilter::Pointer copy = filter->newFilterInstance(true);
      pipeline->pushBack(copy);
    }

  }
  pipeline->addMessageReceiver(m_PipelineMessageObserver);
  return pipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::updateFavorite(const QString& filePath, const QString& name, QSettings::Format format)
{
  //If the filePath already exists - delete it so that we get a clean write to the file
  QFileInfo fi(filePath);
  if (fi.exists() == true)
  {
    QFile f(filePath);
    if (f.remove() == false)
    {
      QMessageBox::warning ( this, QString::fromLatin1("Favorite Update Error"),
                             QString::fromLatin1("There was an error removing the existing favorite. The favorite was NOT updated.") );
      return;
    }
  }

  // Create a Pipeline Object and fill it with the filters from this View
  FilterPipeline::Pointer pipeline = getFilterPipeline();
  int err = QFilterParametersWriter::WritePipelineToFile(pipeline, fi.absoluteFilePath(), name, format, reinterpret_cast<IObserver*>(m_PipelineMessageObserver));
  if (err < 0)
  {
    m_StatusBar->showMessage(tr("There was an error while updating the favorite '%1'.").arg(name));
  }
  else
  {
    m_StatusBar->showMessage(tr("Favorite '%1' has been updated successfully.").arg(name));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::writePipeline(QString filePath)
{
  //If the filePath already exists - delete it so that we get a clean write to the file
  QFileInfo fi(filePath);
  if (fi.exists() == true)
  {
    QFile f(filePath);
    if (f.remove() == false)
    {
      QMessageBox::warning(this, QString::fromLatin1("Pipeline Write Error"),
        QString::fromLatin1("There was an error removing the existing pipeline file. The pipeline was NOT saved."));
      return;
    }
  }

  QString ext = fi.completeSuffix();
  QString name = fi.fileName();

  // Create a Pipeline Object and fill it with the filters from this View
  FilterPipeline::Pointer pipeline = getFilterPipeline();

  int err = 0;
  if (ext == "ini" || ext == "txt")
  {
    err = QFilterParametersWriter::WritePipelineToFile(pipeline, fi.absoluteFilePath(), name, QSettings::IniFormat, reinterpret_cast<IObserver*>(m_PipelineMessageObserver));
  }
  else if (ext == "dream3d")
  {
    err = H5FilterParametersWriter::WritePipelineToFile(pipeline, fi.absoluteFilePath(), name, reinterpret_cast<IObserver*>(m_PipelineMessageObserver));
  }
  else if (ext == "json")
  {
    err = JsonFilterParametersWriter::WritePipelineToFile(pipeline, fi.absoluteFilePath(), name, reinterpret_cast<IObserver*>(m_PipelineMessageObserver));
  }
  else
  {
    m_StatusBar->showMessage(tr("The pipeline was not written to file '%1'. '%2' is an unsupported file extension.").arg(name).arg(ext));
    return;
  }

  if (err < 0)
  {
    m_StatusBar->showMessage(tr("There was an error while saving the pipeline to file '%1'.").arg(name));
  }
  else
  {
    m_StatusBar->showMessage(tr("The pipeline has been saved successfully to '%1'.").arg(name));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineViewWidget::openPipeline(const QString &filePath, ExtractionType type)
{
  //If the filePath already exists - delete it so that we get a clean write to the file
  QFileInfo fi(filePath);
  if (fi.exists() == false)
  {
      QMessageBox::warning(this, QString::fromLatin1("Pipeline Read Error"),
        QString::fromLatin1("There was an error opening the specified pipeline file. The pipeline file does not exist."));
      return -1;
  }

  // Clear the pipeline Issues table first so we can collect all the error messages
  emit pipelineIssuesCleared();

  QString ext = fi.suffix();
  QString name = fi.fileName();

  // Read the pipeline from the file
  FilterPipeline::Pointer pipeline = readPipelineFromFile(filePath);

  // Check that a valid extension was read...
  if (pipeline == FilterPipeline::NullPointer())
  {
    m_StatusBar->showMessage(tr("The pipeline was not read correctly from file '%1'. '%2' is an unsupported file extension.").arg(name).arg(ext));
    return -1;
  }

  // Choose whether to append, replace, or prepend existing pipeline
  if (type == Append)
  {
    populatePipelineView(pipeline, Append);
  }
  else if (type == Replace)
  {
    populatePipelineView(pipeline, Replace);
  }
  else
  {
    populatePipelineView(pipeline, Prepend);
  }

  // Notify user of successful read
  m_StatusBar->showMessage(tr("The pipeline has been read successfully from '%1'.").arg(name));

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer PipelineViewWidget::readPipelineFromFile(const QString &filePath)
{
  QFileInfo fi(filePath);
  QString ext = fi.suffix();
  QString name = fi.fileName();

  FilterPipeline::Pointer pipeline;
  if (ext == "ini" || ext == "txt")
  {
    pipeline = QFilterParametersReader::ReadPipelineFromFile(filePath, QSettings::IniFormat, dynamic_cast<IObserver*>(m_PipelineMessageObserver));
  }
  else if (ext == "dream3d")
  {
    pipeline = H5FilterParametersReader::ReadPipelineFromFile(filePath);
  }
  else if (ext == "json")
  {
    pipeline = JsonFilterParametersReader::ReadPipelineFromFile(filePath);
  }
  else
  {
    pipeline = FilterPipeline::NullPointer();
  }

  return pipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::addFilter(const QString& filterClassName, int index)
{
  if (this->isEnabled() == false) { return; }
  FilterManager* fm = FilterManager::Instance();
  if(NULL == fm) { return; }
  IFilterFactory::Pointer wf = fm->getFactoryForFilter(filterClassName);
  if (NULL == wf.get()) { return; }

  // Create an instance of the filter. Since we are dealing with the AbstractFilter interface we can not
  // actually use the concrete filter class. We are going to have to rely on QProperties or Signals/Slots
  // to communicate changes back to the filter.
  AbstractFilter::Pointer filter = wf->create();

  if (index < 0) // If the programmer wants to add it to the end of the list
  {
    index = filterCount() - 1; // filterCount will come back with the vertical spacer, and if index is still
    // -1 then the spacer is not there and it will get added so the next time through. this should work
  }

  // Create a FilterWidget object
  PipelineFilterWidget* w = new PipelineFilterWidget(filter, NULL, this);

  // Add the filter widget to this view widget
  addFilterWidget(w, index);

  // Clear the pipeline Issues table first so we can collect all the error messages
  emit pipelineIssuesCleared();
  // Now preflight the pipeline for this filter.
  preflightPipeline();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::addFilterWidget(PipelineFilterWidget* w, int index)
{
  bool addSpacer = false;
  if (filterCount() <= 0)
  {
    if (NULL != m_EmptyPipelineLabel)
    {
      m_EmptyPipelineLabel->hide();
      delete m_EmptyPipelineLabel;
      m_EmptyPipelineLabel = NULL;
    }
    QLayout* l = layout();
    if (NULL != l)
    {
      qDeleteAll(l->children());
      delete l;
    }

    m_FilterWidgetLayout = new QVBoxLayout(this);
    m_FilterWidgetLayout->setObjectName(QString::fromUtf8("m_FilterWidgetLayout"));
    m_FilterWidgetLayout->setContentsMargins(2, 2, 2, 2);
    m_FilterWidgetLayout->setSpacing(3);
    addSpacer = true;

    if(index < 0)
    {
      index = 0;
    }
  }

  // The layout will take control of the PipelineFilterWidget 'w' instance
  m_FilterWidgetLayout->insertWidget(index, w);
  // Set the Parent
  w->setParent(this);

  if(index == -1)
  {
    index = filterCount() - 1;
  }

  /// Now setup all the connections between the various widgets

  // When the filter is removed from this view
  connect(w, SIGNAL(filterWidgetRemoved(PipelineFilterWidget*)),
          this, SLOT(removeFilterWidget(PipelineFilterWidget*)) );

  // When the FilterWidget is selected
  connect(w, SIGNAL(widgetSelected(PipelineFilterWidget*)),
          this, SLOT(setSelectedFilterWidget(PipelineFilterWidget*)) );

  // When the filter widget is dragged
  connect(w, SIGNAL(dragStarted(PipelineFilterWidget*)),
          this, SLOT(setFilterBeingDragged(PipelineFilterWidget*)) );

  connect(w, SIGNAL(parametersChanged()),
          this, SLOT(preflightPipeline()));

  // Check to make sure at least the vertical spacer is in the Layout
  if (addSpacer)
  {
    QSpacerItem* verticalSpacer = new QSpacerItem(20, 361, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_FilterWidgetLayout->insertSpacerItem(-1, verticalSpacer);
  }

  // Make sure the widget titles are all correct
  reindexWidgetTitles();

  // Finally, set this new filter widget as selected in order to show the input parameters right away
  w->setIsSelected(true);
  // Get the filter to ignore Scroll Wheel Events
  w->installEventFilter( this);

  // Emit that the pipeline changed
  emit pipelineChanged();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineViewWidget::eventFilter(QObject* o, QEvent* e)
{
  if ( e->type() == QEvent::Wheel && qobject_cast<PipelineFilterWidget*>(o) )
  {
    return false;
  }
  return QFrame::eventFilter( o, e );
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::preflightPipeline()
{
  emit pipelineIssuesCleared();
  // Create a Pipeline Object and fill it with the filters from this View
  FilterPipeline::Pointer pipeline = getFilterPipeline();

  FilterPipeline::FilterContainerType filters = pipeline->getFilterContainer();
  for(int i = 0; i < filters.size(); i++)
  {
    PipelineFilterWidget* fw = filterWidgetAt(i);
    if (fw)
    {
      fw->setHasPreflightErrors(false);
    }
    filters.at(i)->setErrorCondition(0);
  }


  QProgressDialog progress("Preflight Pipeline", "", 0, 1, this);
  progress.setWindowModality(Qt::WindowModal);

  // Preflight the pipeline
  int err = pipeline->preflightPipeline();
  if (err < 0)
  {
    //FIXME: Implement this
  }
  progress.setValue(1);

  int count = pipeline->getFilterContainer().size();
  //Now that the preflight has been executed loop through the filters and check their error condition and set the
  // outline on the filter widget if there were errors or warnings
  for(qint32 i = 0; i < count; ++i)
  {
    PipelineFilterWidget* fw = filterWidgetAt(i);
    if (fw)
    {
      AbstractFilter::Pointer filter = fw->getFilter();
      if(filter->getErrorCondition() < 0) {fw->setHasPreflightErrors(true);}
    }
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::removeFilterWidget(PipelineFilterWidget* whoSent)
{
  if (whoSent)
  {
    QWidget* w = qobject_cast<QWidget*>(whoSent);

    m_FilterWidgetLayout->removeWidget(w);
    if (m_SelectedFilterWidget == w)
    {
      m_SelectedFilterWidget = NULL;
    }
    if (w)
    {
      whoSent->getFilter()->setPreviousFilter(AbstractFilter::NullPointer());
      whoSent->getFilter()->setNextFilter(AbstractFilter::NullPointer());
      w->deleteLater();
    }
  }

  QSpacerItem* spacer = m_FilterWidgetLayout->itemAt(0)->spacerItem();
  if (NULL != spacer)
  {
    m_FilterWidgetLayout->removeItem(spacer);
  }

  reindexWidgetTitles();
  preflightPipeline();

  resetLayout();
  emit pipelineChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setFilterBeingDragged(PipelineFilterWidget* w)
{
  m_FilterBeingDragged = w;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setSelectedFilterWidget(PipelineFilterWidget* w)
{
  if(NULL != m_SelectedFilterWidget && w != m_SelectedFilterWidget)
  {
    m_SelectedFilterWidget->setIsSelected(false);
  }
  m_SelectedFilterWidget = w;

  emit filterInputWidgetChanged(w->getFilterInputWidget());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setStatusBar(QStatusBar* statusBar)
{
  m_StatusBar = statusBar;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::dragEnterEvent( QDragEnterEvent* event)
{
  event->acceptProposedAction();
#if 0

  PipelineFilterWidget* w = qobject_cast<PipelineFilterWidget*>(childAt(event->pos()));
  if (w != NULL)
  {
    qDebug() << "PipelineFilterWidget Found: " << w->getFilter()->getNameOfClass() << "\n";
  }
  QVBoxLayout* l = qobject_cast<QVBoxLayout*>(childAt(event->pos()));
  if (l != NULL)
  {
    qDebug() << "PipelineViewWidget::dragEnterEvent: Found the QVBoxLayout" << "\n";
  }
  PipelineViewWidget* o = qobject_cast<PipelineViewWidget*>(childAt(event->pos()));
  if (o != NULL)
  {
    qDebug() << "PipelineViewWidget::dragEnterEvent: " << o->objectName() << "\n";
  }
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::addDREAM3DReaderFilter(const QString& filePath, ExtractionType type)
{
  DataContainerReader::Pointer reader = DataContainerReader::New();
  reader->setInputFile(filePath);

  switch(type)
  {
    case Replace:
    {
      clearWidgets();
      // Do not place a break statement here - Replace needs to use the "Append" code below.
    }
    case Append:
    {
      // Create a PipelineFilterWidget using the current AbstractFilter instance to initialize it
      PipelineFilterWidget* w = new PipelineFilterWidget(reader, NULL, this);
      int index = filterCount() - 1; // We want to add the filter as the next filter but BEFORE the vertical spacer
      addFilterWidget(w, index);
      break;
    }
    case Prepend:
    {
      // Prepend filter to pipeline
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::populatePipelineView(FilterPipeline::Pointer pipeline, ExtractionType type)
{
  if (NULL == pipeline.get()) { clearWidgets(); return; }
  // Clear the Pipeline First
  if (type == Replace) { clearWidgets(); }
  // get a reference to the filters which are in some type of container object.
  FilterPipeline::FilterContainerType& filters = pipeline->getFilterContainer();
  int fCount = filters.size();

  int index = -1;

  // QProgressDialog progress("Opening Pipeline File....", "Cancel", 0, fCount, this);
  // progress.setWindowModality(Qt::WindowModal);
  // progress.setMinimumDuration(2000);
  PipelineFilterWidget* firstWidget = NULL;
  // Start looping on each filter
  for (int i = 0; i < fCount; i++)
  {
    //   progress.setValue(i);
    // Create a PipelineFilterWidget using the current AbstractFilter instance to initialize it
    PipelineFilterWidget* w = new PipelineFilterWidget(filters.at(i), NULL, this);
    if (type == Replace || type == Append)
    {
      index = filterCount() - 1; // We want to add the filter as the next filter but BEFORE the vertical spacer
    }
    else
    {
      index++;
    }

    addFilterWidget(w, index);
    if(i == 0) { firstWidget = w; }

  }
  if (firstWidget) { firstWidget->setIsSelected(true); }
  // progress.setValue(fCount);


  // Now preflight the pipeline for this filter.
  preflightPipeline();
  if (type == Append || type == Prepend) { emit pipelineChanged(); }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::dragMoveEvent(QDragMoveEvent* event)
{
	m_LastDragPoint = event->pos();

	// If cursor is within margin boundaries, start scrolling
	if (shouldAutoScroll(event->pos()))
	{
		startAutoScroll();
	}
	// Otherwise, stop scrolling
	else
	{
		stopAutoScroll();
	}

	QObject* o = qobject_cast<QObject*>(childAt(event->pos()));
	if (o == NULL && event->dropAction() == Qt::MoveAction) // WE ONLY deal with this if the user is moving an existing pipeline filter
	{
		int count = filterCount();
		for (int i = 0; i < count; ++i)
		{
			PipelineFilterWidget* w = qobject_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(i)->widget());
			if (w != NULL && m_FilterBeingDragged != NULL && w != m_FilterBeingDragged)
			{
				if (event->pos().y() < w->geometry().y())
				{
					m_FilterWidgetLayout->removeWidget(m_FilterBeingDragged);
					break;
				}
			}
		}
		bool didInsert = false;
		count = filterCount();
		for (int i = 0; i < count; ++i)
		{
			PipelineFilterWidget* w = qobject_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(i)->widget());
			if (w != NULL && m_FilterBeingDragged != NULL && w != m_FilterBeingDragged)
			{
				/*if (event->pos().y() <= 0)
				{
				m_FilterWidgetLayout->insertWidget(draggedWidgetIndex, m_FilterBeingDragged);
				setSelectedFilterWidget(m_FilterBeingDragged);
				didInsert = true;
				break;
				}*/
				if (event->pos().y() < w->geometry().y())
				{
					m_FilterWidgetLayout->insertWidget(i, m_FilterBeingDragged);
					setSelectedFilterWidget(m_FilterBeingDragged);
					reindexWidgetTitles();
					didInsert = true;
					break;
				}
			}
		}
		// Check to see if we are trying to move it to the end
		if (false == didInsert && count > 0)
		{
			PipelineFilterWidget* w = qobject_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(count - 2)->widget());
			if (w != NULL && m_FilterBeingDragged != NULL && w != m_FilterBeingDragged)
			{
				if (event->pos().y() > w->geometry().y() + w->geometry().height())
				{
					m_FilterWidgetLayout->insertWidget(count - 2, m_FilterBeingDragged);
					setSelectedFilterWidget(m_FilterBeingDragged);
					reindexWidgetTitles();
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::dropEvent(QDropEvent* event)
{

  if (event->mimeData()->hasUrls())
  {
    QList<QUrl> urlList;
    QString fName;
    urlList = event->mimeData()->urls(); // returns list of QUrls
    // if just text was dropped, urlList is empty (size == 0)

    if ( urlList.size() > 0) // if at least one QUrl is present in list
    {
      fName = urlList[0].toLocalFile(); // convert first QUrl to local path
      fName = QDir::toNativeSeparators(fName);
      QFileInfo fi(fName);

      FileDragMessageBox* msgBox = new FileDragMessageBox(this, filterCount());
      msgBox->setFilePath(fName);
      connect(msgBox, SIGNAL(fireExtractPipelineFromFile(const QString&, ExtractionType)), this, SLOT(openPipeline(const QString&, ExtractionType)));
      connect(msgBox, SIGNAL(fireAddDREAM3DReaderFilter(const QString&, ExtractionType)), this, SLOT(addDREAM3DReaderFilter(const QString&, ExtractionType)));

      if(fi.suffix().endsWith("dream3d") == false)
      {
        msgBox->getAddFilterBtn()->hide();
        msgBox->getExtractPipelineBtn()->hide();
        msgBox->getDescriptionLabel()->hide();
      }

      msgBox->exec();
      msgBox->deleteLater();
    }
  }
  else if(m_FilterBeingDragged != NULL && event->dropAction() == Qt::MoveAction)
  {
    // This path is take if a filter is being dragged around in the pipeline and dropped.
    setSelectedFilterWidget(m_FilterBeingDragged);
    m_FilterBeingDragged = NULL;
    // Make sure the widget titles are all correct
    reindexWidgetTitles();
    preflightPipeline();
  }
  else
  {
    const QMimeData* mimedata = event->mimeData();
    QByteArray dropData = mimedata->data("text/plain");
    // This path is taken if a filter is dropped from the list of filters
    if(mimedata->hasText())
    {
      QString name(dropData);
      // We need to figure out where it was dropped relative to other filters
      int count = filterCount() - 1;
      for (int i = 0; i < count; ++i)
      {
        PipelineFilterWidget* w = qobject_cast<PipelineFilterWidget*>(m_FilterWidgetLayout->itemAt(i)->widget());
        if(w != NULL)
        {
          if(event->pos().y() < w->geometry().y())
          {
            count = i;
            break;
          }
        }
      }
      // Now that we have an index, insert the filter.
      addFilter(name, count);
    }
  }
  event->acceptProposedAction();

  // Stop auto scrolling if widget is dropped
  stopAutoScroll();

  emit pipelineChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::stopAutoScroll()
{
  m_autoScrollTimer.stop();
  m_autoScrollCount = 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::startAutoScroll()
{
  int scrollInterval = 50;
  m_autoScrollTimer.start(scrollInterval);
  m_autoScrollCount = 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::doAutoScroll()
{
  // find how much we should scroll with
  int verticalStep = m_ScrollArea->verticalScrollBar()->pageStep();
  int horizontalStep = m_ScrollArea->horizontalScrollBar()->pageStep();
  if (m_autoScrollCount < qMax(verticalStep, horizontalStep))
  { m_autoScrollCount = m_autoScrollCount + 15; }

  int margin = m_AutoScrollMargin;
  int verticalValue = m_ScrollArea->verticalScrollBar()->value();
  int horizontalValue = m_ScrollArea->horizontalScrollBar()->value();

  QPoint pos = m_ScrollArea->viewport()->mapFromGlobal(QCursor::pos());
  QRect area = m_ScrollArea->geometry();

  // do the scrolling if we are in the scroll margins
  if (pos.y() - area.top() < margin)
  { m_ScrollArea->verticalScrollBar()->setValue(verticalValue - m_autoScrollCount); }
  else if (area.bottom() - pos.y() < margin)
  { m_ScrollArea-> verticalScrollBar()->setValue(verticalValue + m_autoScrollCount); }
  //  if (pos.x() - area.left() < margin)
  //    m_ScrollArea->horizontalScrollBar()->setValue(horizontalValue - d->m_autoScrollCount);
  //  else if (area.right() - pos.x() < margin)
  //    m_ScrollArea->horizontalScrollBar()->setValue(horizontalValue + d->m_autoScrollCount);
  // if nothing changed, stop scrolling
  bool verticalUnchanged = (verticalValue == m_ScrollArea->verticalScrollBar()->value());
  bool horizontalUnchanged = (horizontalValue == m_ScrollArea->horizontalScrollBar()->value());
  if (verticalUnchanged && horizontalUnchanged)
  {
    stopAutoScroll();
  }
  else
  {
    m_ScrollArea->viewport()->update();
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineViewWidget::shouldAutoScroll(const QPoint& pos)
{
  if (NULL == m_ScrollArea) { return false; }
  QPoint scpos = m_ScrollArea->viewport()->mapFromGlobal(QCursor::pos());
  QRect rect = m_ScrollArea->geometry();

  if (scpos.y() <= getAutoScrollMargin())
  {
    return true;
  }
  else if (pos.y() >= rect.height() - getAutoScrollMargin())
  {
    return true;
  }
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::setContextMenuActions(QList<QAction*> list)
{
  m_MenuActions = list;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::on_customContextMenuRequested(const QPoint& pos)
{
  // Note: We must map the point to global from the viewport to
  // account for the header.
  showContextMenu(mapToGlobal(pos) );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewWidget::showContextMenu(const QPoint& globalPos)
{
  m_Menu.clear();
  for (int i = 0; i < m_MenuActions.size(); i++)
  {
    m_Menu.addAction(m_MenuActions[i]);
  }
  m_Menu.exec(globalPos);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStatusBar* PipelineViewWidget::getStatusBar()
{
  return m_StatusBar;
}



