// This file is part of the Acts project.
//
// Copyright (C) 2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ActsExamples/MagneticField/MagneticField.hpp"

#include "Acts/Definitions/Units.hpp"
#include "Acts/MagneticField/BFieldMapUtils.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/MagneticField/NullBField.hpp"
#include "Acts/MagneticField/SolenoidBField.hpp"
#include "Acts/Plugins/Python/Utilities.hpp"
#include "ActsExamples/MagneticField/FieldMapRootIo.hpp"
#include "ActsExamples/MagneticField/FieldMapTextIo.hpp"

#include "Acts/MagneticField/RestrictedBField.hpp"
#include "Acts/MagneticField/RestrictedBFieldCylindrical.hpp"
#include "Acts/MagneticField/RestrictedBFieldCylindrical2.hpp"
#include "Acts/MagneticField/RestrictedBFieldCylindrical3.hpp"

//#include "Acts/MagneticField/RestrictedBFieldIndu2.hpp"

#include "Acts/MagneticField/RestrictedBField_Samu.hpp"
#include "Acts/MagneticField/RestrictedBField_Samu_red.hpp"

#include "Acts/MagneticField/RestrictedBFieldIndu2.hpp"
#include "Acts/MagneticField/RestrictedBFieldIndu3.hpp"
#include "Acts/MagneticField/RestrictedBFieldIndu4.hpp"
#include "Acts/MagneticField/RestrictedBFieldIndu3_2m.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace pybind11::literals;

namespace Acts::Python {

void addMagneticField(Context& ctx) {
  auto [m, mex, prop] = ctx.get("main", "examples", "propagation");

  py::class_<Acts::MagneticFieldProvider,
             std::shared_ptr<Acts::MagneticFieldProvider>>(
      m, "MagneticFieldProvider");

  py::class_<Acts::InterpolatedMagneticField,
             std::shared_ptr<Acts::InterpolatedMagneticField>>(
      m, "InterpolatedMagneticField");

  m.def("solenoidFieldMap", &Acts::solenoidFieldMap, py::arg("rlim"),
        py::arg("zlim"), py::arg("nbins"), py::arg("field"));

  py::class_<Acts::ConstantBField, Acts::MagneticFieldProvider,
             std::shared_ptr<Acts::ConstantBField>>(m, "ConstantBField")
      .def(py::init<Acts::Vector3>());

  py::class_<RestrictedBField, Acts::MagneticFieldProvider, std::shared_ptr<RestrictedBField>>(m, "RestrictedBField")
      .def(py::init<Acts::Vector3>());

  py::class_<RestrictedBFieldCylindrical, Acts::MagneticFieldProvider, std::shared_ptr<RestrictedBFieldCylindrical>>(m, "RestrictedBFieldCylindrical")
      .def(py::init<Acts::Vector3>());


  //Industrial magnet

  // py::class_<RestrictedBFieldIndu2, Acts::MagneticFieldProvider, std::shared_ptr<RestrictedBFieldIndu2>>(m, "RestrictedBFieldIndu2")
  //     .def(py::init<Acts::Vector3>());

  //Samurai Baseline 1x3x4 m

  py::class_<RestrictedBField_Samu, Acts::MagneticFieldProvider, std::shared_ptr<RestrictedBField_Samu>>(m, "RestrictedBField_Samu")
      .def(py::init<Acts::Vector3>());

  //Samurai Baseline reduced 2.6m width

  py::class_<RestrictedBField_Samu_red, Acts::MagneticFieldProvider, std::shared_ptr<RestrictedBField_Samu_red>>(m, "RestrictedBField_Samu_red")
      .def(py::init<Acts::Vector3>());

  //Industrial magnet 1.6m 2 Crystall pull

  py::class_<RestrictedBFieldIndu2, Acts::MagneticFieldProvider, std::shared_ptr<RestrictedBFieldIndu2>>(m, "RestrictedBFieldIndu2")
      .def(py::init<Acts::Vector3>());

  //Industrial magnet 1.6m 3 Crystall pull

  py::class_<RestrictedBFieldIndu3, Acts::MagneticFieldProvider, std::shared_ptr<RestrictedBFieldIndu3>>(m, "RestrictedBFieldIndu3")
      .def(py::init<Acts::Vector3>());

  //Industrial magnet 1.6m 4 Crystall pull

  py::class_<RestrictedBFieldIndu4, Acts::MagneticFieldProvider, std::shared_ptr<RestrictedBFieldIndu4>>(m, "RestrictedBFieldIndu4")
      .def(py::init<Acts::Vector3>());

  //Industrial magnet 2m 3 Crystall pull

  py::class_<RestrictedBFieldIndu3_2m, Acts::MagneticFieldProvider, std::shared_ptr<RestrictedBFieldIndu3_2m>>(m, "RestrictedBFieldIndu3_2m")
      .def(py::init<Acts::Vector3>());

  py::class_<ActsExamples::detail::InterpolatedMagneticField2,
             Acts::InterpolatedMagneticField, Acts::MagneticFieldProvider,
             std::shared_ptr<ActsExamples::detail::InterpolatedMagneticField2>>(
      mex, "InterpolatedMagneticField2");

  py::class_<ActsExamples::detail::InterpolatedMagneticField3,
             Acts::InterpolatedMagneticField, Acts::MagneticFieldProvider,
             std::shared_ptr<ActsExamples::detail::InterpolatedMagneticField3>>(
      mex, "InterpolatedMagneticField3");

  py::class_<Acts::NullBField, Acts::MagneticFieldProvider,
             std::shared_ptr<Acts::NullBField>>(m, "NullBField")
      .def(py::init<>());

  {
    using Config = Acts::SolenoidBField::Config;

    auto sol =
        py::class_<Acts::SolenoidBField, Acts::MagneticFieldProvider,
                   std::shared_ptr<Acts::SolenoidBField>>(m, "SolenoidBField")
            .def(py::init<Config>())
            .def(py::init([](double radius, double length, std::size_t nCoils,
                             double bMagCenter) {
                   return Acts::SolenoidBField{
                       Config{radius, length, nCoils, bMagCenter}};
                 }),
                 py::arg("radius"), py::arg("length"), py::arg("nCoils"),
                 py::arg("bMagCenter"));

    py::class_<Config>(sol, "Config")
        .def(py::init<>())
        .def_readwrite("radius", &Config::radius)
        .def_readwrite("length", &Config::length)
        .def_readwrite("nCoils", &Config::nCoils)
        .def_readwrite("bMagCenter", &Config::bMagCenter);
  }

  mex.def(
      "MagneticFieldMapXyz",
      [](const std::string& filename, const std::string& tree,
         double lengthUnit, double BFieldUnit, bool firstOctant) {
        const std::filesystem::path file = filename;

        auto mapBins = [](std::array<std::size_t, 3> bins,
                          std::array<std::size_t, 3> sizes) {
          return (bins[0] * (sizes[1] * sizes[2]) + bins[1] * sizes[2] +
                  bins[2]);
        };

        if (file.extension() == ".root") {
          auto map = ActsExamples::makeMagneticFieldMapXyzFromRoot(
              std::move(mapBins), file.native(), tree, lengthUnit, BFieldUnit,
              firstOctant);
          return std::make_shared<
              ActsExamples::detail::InterpolatedMagneticField3>(std::move(map));
        } else if (file.extension() == ".txt") {
          auto map = ActsExamples::makeMagneticFieldMapXyzFromText(
              std::move(mapBins), file.native(), lengthUnit, BFieldUnit,
              firstOctant);
          return std::make_shared<
              ActsExamples::detail::InterpolatedMagneticField3>(std::move(map));
        } else {
          throw std::runtime_error("Unsupported magnetic field map file type");
        }
      },
      py::arg("file"), py::arg("tree") = "bField",
      py::arg("lengthUnit") = Acts::UnitConstants::mm,
      py::arg("BFieldUnit") = Acts::UnitConstants::T,
      py::arg("firstOctant") = false);

  mex.def(
      "MagneticFieldMapRz",
      [](const std::string& filename, const std::string& tree,
         double lengthUnit, double BFieldUnit, bool firstQuadrant) {
        const std::filesystem::path file = filename;

        auto mapBins = [](std::array<std::size_t, 2> bins,
                          std::array<std::size_t, 2> sizes) {
          return (bins[1] * sizes[0] + bins[0]);
        };

        if (file.extension() == ".root") {
          auto map = ActsExamples::makeMagneticFieldMapRzFromRoot(
              std::move(mapBins), file.native(), tree, lengthUnit, BFieldUnit,
              firstQuadrant);
          return std::make_shared<
              ActsExamples::detail::InterpolatedMagneticField2>(std::move(map));
        } else if (file.extension() == ".txt") {
          auto map = ActsExamples::makeMagneticFieldMapRzFromText(
              std::move(mapBins), file.native(), lengthUnit, BFieldUnit,
              firstQuadrant);
          return std::make_shared<
              ActsExamples::detail::InterpolatedMagneticField2>(std::move(map));
        } else {
          throw std::runtime_error("Unsupported magnetic field map file type");
        }
      },
      py::arg("file"), py::arg("tree") = "bField",
      py::arg("lengthUnit") = Acts::UnitConstants::mm,
      py::arg("BFieldUnit") = Acts::UnitConstants::T,
      py::arg("firstQuadrant") = false);
}

}  // namespace Acts::Python