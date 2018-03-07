#include "PythonBindingClass.h"

#include <QtCore/QDateTime>

#include "CodeScraper/CodeScraperConstants.h"
#include "CodeScraper/SIMPLPyBind11Config.h"
#include "PythonBindingsModule.h"

PythonBindingClass::PythonBindingClass(PythonBindingsModule* moduleCode)
: QObject(nullptr)
, m_NeedsWrapping(false)
, m_HasSuperClass(false)
, m_IsSharedPointer(false)
, m_HasStaticNewMacro(false)
, m_Module(moduleCode)
{
}

PythonBindingClass::~PythonBindingClass() = default;

void PythonBindingClass::addProperty(const QString& property)
{
  m_Properties.push_back(property);
}
void PythonBindingClass::clearProperties()
{
  m_Properties.clear();
}

void PythonBindingClass::addMethod(const QString& method)
{
  m_Methods.push_back(method);
}
void PythonBindingClass::clearMethods()
{
  m_Methods.clear();
}

void PythonBindingClass::addStaticCreation(const QString& method)
{
  m_StaticNewMethods.push_back(method);
}
void PythonBindingClass::clearStaticCreation()
{
  m_StaticNewMethods.clear();
}

void PythonBindingClass::addConstructor(const QString& constructor)
{
  m_Constructors.push_back(constructor);
}
void PythonBindingClass::clearConstructors()
{
  m_Constructors.clear();
}

void PythonBindingClass::addEnumeration(const QString& name, const QStringList& values)
{
  m_Enumerations[name] = values;
}
void PythonBindingClass::clearEnumerations()
{
  m_Enumerations.clear();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PythonBindingClass::writeBindingFile(const QString& outputFilePath)
{
  if(getNeedsWrapping())
  {
    QString output;
    QTextStream out(&output);
    out << generateTopMatterCode();
    out << generateSharedPointerInitCode();
    out << generateStaticNewCode();
    out << generateConstructorsCodes();
    out << generatePropertiesCode();
    out << generateMethodCode();
    out << "  ;\n";
    out << generateEnumerationCode();
    out << generateFooterCode();

    writeOutput(getNeedsWrapping(), output, outputFilePath);

    QString initCode;
    QTextStream init(&initCode);

    init << "PySharedPtrClass<" << getClassName() << "> ";
    init << getLibName() << "_" << getClassName() << " = ";
    if(getHasSuperClass())
    {
      init << "pybind11_init_" << getLibName() << "_" << getClassName() << "(mod, " << getLibName() << "_" << getSuperClass() << ");";
    }
    else
    {
      init << "pybind11_init_" << getLibName() << "_" << getClassName() << "(mod);";
    }

    m_Module->addInitCode(getClassName(), initCode);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString quote(const QString& str)
{
  return QString("\"%1\"").arg(str);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PythonBindingClass::writeOutput(bool didReplace, const QString& outLines, QString filename)
{
  if(didReplace == true)
  {
    QFileInfo fi2(filename);
    QString parentPath = fi2.path();
    QDir dir;
    // std::cout << "Creating Path: " << parentPath.toStdString() << std::endl;
    if(!dir.mkpath(parentPath))
    {
      QString ss = QObject::tr("Error creating parent path '%1'").arg(parentPath);
      qDebug() << ss;
      return;
    }
#if OVERWRITE_SOURCE_FILE
    QFile hOut(filename);
#else
    QString tmpPath = "/tmp/" + fi2.fileName();
    QFile hOut(tmpPath);
#endif
    hOut.open(QFile::WriteOnly);
    QTextStream stream(&hOut);
    stream << outLines;
    hOut.close();

   // qDebug() << "Pybind11 Generated for: " << fi2.absoluteFilePath();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PythonBindingClass::generateTopMatterCode()
{
  QFileInfo fi(m_SourceFile);
  QString subPath = fi.absolutePath();
  subPath = subPath.replace(m_SourceDir.absolutePath(), ""); // Remove the system dependent file path
  subPath = subPath.remove(0, 1);                            // Remove the front / character

  // Create the Top part of the file from a template file
  QFile source(SIMPL::PyBind11::TemplateDir + "/Pybind11TopMatter.txt");
  source.open(QFile::ReadOnly);
  QString headerTemplate = source.readAll();
  source.close();
  headerTemplate = headerTemplate.replace(CLASS_NAME, getClassName());
  QString headerPath = QString("%1/%2.h").arg(subPath).arg(getClassName());
  if(!m_CharsToStrip.isEmpty())
  {
    headerPath = headerPath.replace(m_CharsToStrip, "");
  }

  headerTemplate = headerTemplate.replace(HEADER_PATH, headerPath);
  headerTemplate = headerTemplate.replace(LIB_NAME, m_LibName);
  headerTemplate = headerTemplate.replace("@DATE_TIME_GENERATED@", QDateTime::currentDateTime().toString());
  return headerTemplate;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PythonBindingClass::generateSharedPointerInitCode()
{
  if(!getIsSharedPointer())
  {
    return QString("");
  }
  QFile source;

  if(getHasSuperClass() && !getSuperClass().isEmpty())
  {
    source.setFileName(SIMPL::PyBind11::TemplateDir + "/DerivedSharedPointerClassInit.txt");
  }
  else
  {
    source.setFileName(SIMPL::PyBind11::TemplateDir + "/SharedPointerClassInit.txt");
  }
  source.open(QFile::ReadOnly);
  QString headerTemplate = source.readAll();
  source.close();

  QFileInfo fi(m_SourceFile);
  QString subPath = fi.absolutePath();
  subPath = subPath.replace(m_SourceDir.absolutePath(), ""); // Remove the system dependent file path
  subPath = subPath.remove(0, 1);                            // Remove the front / character

  headerTemplate = headerTemplate.replace(CLASS_NAME, getClassName());
  QString headerPath = QString("%1/%2.h").arg(subPath).arg(getClassName());
  headerTemplate = headerTemplate.replace(HEADER_PATH, headerPath);
  headerTemplate = headerTemplate.replace(SUPERCLASS_NAME, getSuperClass());
  headerTemplate = headerTemplate.replace(LIB_NAME, m_LibName);

//  QString pybind11HeaderPath = QString("%1/pybind11/%2_PY11.h").arg(subPath).arg(getClassName());
  QString pybind11HeaderPath = QString("%1/%2_PY11.h").arg(subPath).arg(getClassName());
  if(!m_CharsToStrip.isEmpty())
  {
    pybind11HeaderPath = pybind11HeaderPath.replace(m_CharsToStrip, "");
  }

  m_Module->addHeader(getClassName(), pybind11HeaderPath);
  m_Module->addDependency(getSuperClass(), getClassName());

  return headerTemplate;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PythonBindingClass::generateStaticNewCode()
{
  //      if(!getIsSharedPointer())
  //      {
  //        return QString("");
  //      }
  QString code;
  QTextStream constructors(&code);

  if(getHasStaticNewMacro())
  {
    constructors << TAB << ".def(py::init([]()\n    { \n      return " << getClassName() << "::New();\n    }\n))" << NEWLINE_SIMPL;
    constructors << TAB << ".def_static(\"New\", &" << getClassName() << "::New)" << NEWLINE_SIMPL;
  }

  for(auto line : m_StaticNewMethods)
  {
    QStringList tokens = line.split("(");
    tokens = tokens[1].replace(")", "").trimmed().split(" ");
    QString methodName = tokens[0];
    if(tokens.size() > 2) // Second Token is ARGS
    {
      QString s1, s2;
      QTextStream o1(&s1);
      QTextStream o2(&s2);
      for(int i = 2; i < tokens.size(); i++)
      {
        o1 << tokens[i] << " var_" << i - 2;
        o2 << "var_" << i - 2;
        if(i != tokens.size() - 1)
        {
          o1 << ", ";
          o2 << ", ";
        }
      }
      constructors << TAB << ".def(py::init([](" << s1 << ") {\n      return " << getClassName() << "::" << methodName << "(" << s2 << ");\n    }))" << NEWLINE_SIMPL;
      constructors << TAB << ".def_static(\"" << methodName << "\", &" << getClassName() << "::" << methodName << ")" << NEWLINE_SIMPL;
    }
  }

  return code;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PythonBindingClass::generateConstructorsCodes()
{
  if(getIsSharedPointer())
  {
    return QString("");
  }

  QString code;
  QTextStream constructors(&code);

  QFile source;
  source.setFileName(SIMPL::PyBind11::TemplateDir + "/SimpleClassInit.txt");
  source.open(QFile::ReadOnly);
  QString headerTemplate = source.readAll();
  source.close();
  headerTemplate = headerTemplate.replace(CLASS_NAME, getClassName());

  QFileInfo fi(m_SourceFile);
  QString subPath = fi.absolutePath();
  subPath = subPath.replace(m_SourceDir.absolutePath(), ""); // Remove the system dependent file path
  subPath = subPath.remove(0, 1);                            // Remove the front / character

  QString headerPath = QString("%1/%2.h").arg(subPath).arg(getClassName());
  headerTemplate = headerTemplate.replace(HEADER_PATH, headerPath);
  headerTemplate = headerTemplate.replace(SUPERCLASS_NAME, getSuperClass());
  headerTemplate = headerTemplate.replace(LIB_NAME, m_LibName);

  constructors << headerTemplate;

  constructors << TAB << ".def(py::init<" << getClassName() << ">())" << NEWLINE_SIMPL;
  constructors << TAB << ".def(py::init<" << getClassName() << " const &>())" << NEWLINE_SIMPL;

  constructors << TAB << "/* Number of costructors: " <<  m_Constructors.size()  << "*/" << NEWLINE_SIMPL;
  for(auto line : m_Constructors)
  {
    QStringList tokens = line.split("(");
    tokens = tokens[1].replace(")", "").trimmed().split(" ");
    QString keyword = tokens[0];
    //.def(py::init<const std::string &>())
    constructors << TAB << ".def(py::init<";
    for(int i = 1; i < tokens.size(); i++)
    {
      constructors << "const " << tokens[i] << " &";
      if(i != tokens.size() - 1) { constructors << ", "; }
    }
    constructors << ">())" << NEWLINE_SIMPL;
  }

  QString pybind11HeaderPath = QString("%1/%2_PY11.h").arg(subPath).arg(getClassName());
  if(!m_CharsToStrip.isEmpty())
  {
    pybind11HeaderPath = pybind11HeaderPath.replace(m_CharsToStrip, "");
  }
  m_Module->addHeader(getClassName(), pybind11HeaderPath);
  m_Module->addDependency(getSuperClass(), getClassName());

  return code;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PythonBindingClass::generatePropertiesCode()
{
  QString code;
  QTextStream out(&code);
  for(auto line : m_Properties)
  {
    QStringList tokens = line.split("(");
    tokens = tokens[1].replace(")", "").trimmed().split(" ");
    QString pyType = tokens[0];
    QString varName = tokens[1];

    // .def_property("name", &Pet::getName, &Pet::setName)  // READ/WRITE
    // def_property_readonly("name", &Pet::getName) // READ ONLY

    if(tokens.size() == 6)
    {
      out << TAB << "/* Property accessors for " << varName << " */" << NEWLINE_SIMPL;
      out << TAB << ".def_property(\"" << varName << "\", &" << getClassName() << "::get" << varName << ", &" << getClassName() << "::set" << varName << ")" << NEWLINE_SIMPL;
    }
    else if(tokens.size() == 4 && tokens[2] == ::kRead)
    {
      out << TAB << "/* Read Only Property for " << varName << " */" << NEWLINE_SIMPL;
      out << TAB << ".def_property_readonly(\"" << varName << "\", &" << getClassName() << "::get" << varName << ")" << NEWLINE_SIMPL;
    }
    else if(tokens.size() == 4 && tokens[2] == ::kWrite)
    {
      out << TAB << "/* Write Only Property for " << varName << " */" << NEWLINE_SIMPL;
      out << TAB << ".def(\"set" << varName << "\", &" << getClassName() << "::set" << varName << ", py::arg(\"" << varName << "\"))" << NEWLINE_SIMPL;
    }
  }
  return code;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PythonBindingClass::generateMethodCode()
{
  QString code;
  QTextStream out(&code);
  for(auto line : m_Methods)
  {
    QStringList tokens = line.split("(");
    tokens = tokens[1].replace(")", "").trimmed().split(" ");
    QString returnType = tokens[0];
    QString methodName = tokens[1];
    out << TAB << "/* Class instance method " << methodName << " */" << NEWLINE_SIMPL;

    if(tokens.size() == 2)
    {
      out << TAB << ".def(\"" << methodName << "\", &" << getClassName() << "::" << methodName << ")" << NEWLINE_SIMPL;
    }
    else if(tokens.size() > 3 && tokens[2] == ::kArgs)
    {
      out << TAB << ".def(\"" << methodName << "\", &" << getClassName() << "::" << methodName;
      for(int32_t i = 3; i < tokens.size(); i++)
      {
        out << ", \n" << TAB << TAB << TAB << TAB << "py::arg(\"" << tokens[i] << "\")";
      }
      out << NEWLINE_SIMPL << TAB << TAB << TAB << ")" << NEWLINE_SIMPL;
    }
  }
  return code;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PythonBindingClass::generateEnumerationCode()
{
  QString code;
  QTextStream out(&code);
  EnumerationType::Iterator iter = m_Enumerations.begin();
  
  for (iter = m_Enumerations.begin(); iter != m_Enumerations.end(); iter++)
  {
    
    out << "\n\n  /* Enumeration code for " << getClassName() << "::" << iter.key() << " ******************/\n";
    QString n = iter.key(); // Get the variable name of the enumeration
    out << "  py::enum_<" << getClassName() << "::" << n << ">(instance, \"" << n << "\")\n";
    QStringList values = iter.value();
    for(auto v : values)
    {
      out << "    .value(\"" << v << "\", " << getClassName() << "::" << n << "::" << v << ")\n";
    }
    out << "    .export_values();\n";
  } 
  //qDebug() << code;
  return code;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PythonBindingClass::generateFooterCode()
{
  QString code;
  QTextStream out(&code);
  //out << TAB << ";" << NEWLINE_SIMPL;
  out << TAB << "return instance;" << NEWLINE_SIMPL;
  out << "}" << NEWLINE_SIMPL;
  out << "\n\n";
 // out << "#endif /* pybind_" << getClassName() << "_H_  */" << NEWLINE_SIMPL;
  return code;
}