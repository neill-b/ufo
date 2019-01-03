/*
 * (C) Copyright 2018-2019 UCAR
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 */

#include "ufo/ObsDomainCheck.h"

#include <string>
#include <vector>

#include "eckit/config/Configuration.h"

#include "ioda/ObsDataVector.h"
#include "ioda/ObsSpace.h"
#include "oops/interface/ObsFilter.h"
#include "oops/util/Logger.h"
#include "oops/util/missingValues.h"
#include "ufo/processWhere.h"
#include "ufo/QCflags.h"
#include "ufo/UfoTrait.h"

namespace ufo {

// -----------------------------------------------------------------------------
static oops::FilterMaker<UfoTrait, oops::ObsFilter<UfoTrait, ObsDomainCheck>>
  mkDomLst_("Domain Check");
// -----------------------------------------------------------------------------

ObsDomainCheck::ObsDomainCheck(ioda::ObsSpace & obsdb, const eckit::Configuration & config)
  : obsdb_(obsdb), config_(config), nogeovals_()
{}

// -----------------------------------------------------------------------------

ObsDomainCheck::~ObsDomainCheck() {}

// -----------------------------------------------------------------------------

void ObsDomainCheck::priorFilter(const GeoVaLs & gv) const {
  const size_t nobs = obsdb_.nlocs();
  const std::string qcgrp = config_.getString("QCname");
  const std::vector<std::string> vars = config_.getStringVector("observed");

  std::vector<bool> inside = processWhere(obsdb_, gv, config_);

  for (size_t jv = 0; jv < vars.size(); ++jv) {
    ioda::ObsDataVector<int> flags(obsdb_, vars[jv], qcgrp);
    int ii = 0;
    for (size_t jobs = 0; jobs < nobs; ++jobs) {
      if (!inside[jobs] && flags[jobs] == 0) flags[jobs] = QCflags::domain;
      if (!inside[jobs]) ++ii;
    }
    flags.save();

    oops::Log::debug() << "ObsDomainCheck: " << obsdb_.obsname() << " " << vars[jv]
                       << " rejected " << ii << " obs." << std::endl;
  }
}

// -----------------------------------------------------------------------------

void ObsDomainCheck::print(std::ostream & os) const {
  os << "ObsDomainCheck: config = " << config_ << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace ufo
