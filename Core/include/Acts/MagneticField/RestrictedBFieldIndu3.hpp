// This file is part of the Acts project.
//
// Copyright (C) 2016-2018 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#pragma once
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"
#include "Acts/Geometry/Volume.hpp"
#include "Acts/Geometry/CuboidVolumeBuilder.hpp"

#include "Acts/Geometry/CuboidVolumeBounds.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/TrackingVolume.hpp"

#include "Acts/Plugins/Json/AlgebraJsonConverter.hpp"
#include "Acts/Utilities/UnitVectors.hpp"
#include "Acts/Utilities/BinningType.hpp"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#include <utility>
#include <stdexcept>
#include <array>
#include <memory>
#include <vector>


namespace Acts {

/// @ingroup MagneticField
/// @brief Constant magnetic field provider
class RestrictedBFieldIndu3 final : public MagneticFieldProvider {
 public:
  struct Cache {
    /// @brief constructor with context
    Cache(const MagneticFieldContext& /*mcfg*/) {}
  };

  /// @brief Default constructor
  RestrictedBFieldIndu3( const Vector3& field)
      :  m_BField(field) {}


      // // std::shared_ptr<TrackingVolume> m_magVolume = TrackingVolume::create(Transform3(Translation3{13500., 0., 0.}),std::make_shared<const CylinderVolumeBounds>(0.0, 1500., 500.));
      // std::shared_ptr<TrackingVolume> m_magVolume = TrackingVolume::create(Transform3(Translation3{13500., 0., 0.}*rotation),std::make_shared<const CylinderVolumeBounds>(0.0, 1500., 500.));
 
//Acts::RotationMatrix3 rotation << 0, 0, -1, 0, 1, 0,1, 0, 0;
  //Acts::RotationMatrix3 rotation = SquareMatrix3(0,0,-1,0,1,0,1,0,0);

  // rotation.col(0) = Acts::Vector3(0, 0, -1);
  // rotation.col(1) = Acts::Vector3(0, 1, 0);
  // rotation.col(2) = Acts::Vector3(1, 0, 0);

  // Create a translation vector.
  Acts::Translation3 translation_ = Acts::Translation3(13600., 0., 0.);

  // Combine rotation and translation into a single transformation.
  //Acts::Transform3 transform = Acts::Transform3(rotation) * Acts::Translation3(translation_); 

  std::shared_ptr<TrackingVolume> m_magVolume = TrackingVolume::create(Transform3(translation_*AngleAxis3(3.14*0.5, Vector3(0., 1., 0.))),std::make_shared<const CylinderVolumeBounds>(0.0, 990., 2100.));

  // std::cout << "Transformation Matrix: \n" << translation_*AngleAxis3(3.14*0.5, Vector3(1., 0., 0.)) << std::endl;

  /// @copydoc MagneticFieldProvider::getField(const Vector3&,MagneticFieldProvider::Cache&) const
  Result<Vector3> getField(const Vector3& position,
                           MagneticFieldProvider::Cache& cache) const override {
    (void)cache;
    if (m_magVolume->inside(position)) {
      return Result<Vector3>::success(m_BField);
    } else {
      return Result<Vector3>::success(Vector3::Zero());
    }
  }

  /// @copydoc MagneticFieldProvider::getFieldGradient(const Vector3&,ActsMatrix<3,3>&,MagneticFieldProvider::Cache&) const
  Result<Vector3> getFieldGradient(
      const Vector3& position, ActsMatrix<3, 3>& derivative,
      MagneticFieldProvider::Cache& cache) const override {
    (void)position;
    (void)derivative;
    (void)cache;
    return Result<Vector3>::success(m_BField);
  }

  /// @copydoc MagneticFieldProvider::makeCache(const MagneticFieldContext&) const
  Acts::MagneticFieldProvider::Cache makeCache(
      const Acts::MagneticFieldContext& mctx) const override {
    return Acts::MagneticFieldProvider::Cache::make<Cache>(mctx);
  }

  /// @brief check whether given 3D position is inside look-up domain
  ///
  /// @param [in] position global 3D position
  /// @return @c true if position is inside the defined look-up grid,
  ///         otherwise @c false
  bool isInside(const Vector3& position) const {
    return m_magVolume->inside(position);
  }

 private:
  // /// magnetic field volume
  // std::shared_ptr<Volume> m_magVolume;
  /// magnetic field vector
  const Vector3 m_BField;
};
}  // namespace Acts
