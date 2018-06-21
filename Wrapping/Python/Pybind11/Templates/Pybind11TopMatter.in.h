#pragma once

/* Template file used is called Pybind11TopMatter.txt */

/**
 * @brief This header file is genererated from a program. Make changes with caution.
 *
 *
 * This header file wraps the SIMPL class @CLASS_NAME@ located at
 * @HEADER_PATH@.
 * The Python bindings are created using Pybind11.
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include "@HEADER_PATH@"

namespace
{ /* Anonymous namespace */
