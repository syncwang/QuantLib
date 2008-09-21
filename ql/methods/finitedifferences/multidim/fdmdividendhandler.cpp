/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmdividendhandler.cpp
    \brief dividend handler for fdm method for one equity direction
*/

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/methods/finitedifferences/multidim/fdmdividendhandler.hpp>

namespace QuantLib {

    FdmDividendHandler::FdmDividendHandler(
         const std::vector<Time> & dividendTimes,
         const std::vector<Real> & dividends,
         const boost::shared_ptr<FdmMesher> & mesher,
         Size equityDirection)
    : x_(mesher->layout()->dim()[equityDirection]),
      dividendTimes_(dividendTimes),
      dividends_(dividends),
      mesher_(mesher),
      equityDirection_(equityDirection) {
        QL_REQUIRE(dividendTimes.size() == dividends.size(), 
                   "incorrect dimensions");

        Array tmp = mesher_->locations(equityDirection);
        for (Size i = 0; i < x_.size(); ++i) {
            x_[i] = std::exp(tmp[i]);
        }
    }

    void FdmDividendHandler::applyTo(Array& a, Time t) const {
        Array aCopy(a);

        std::vector<Time>::const_iterator iter 
            = std::find(dividendTimes_.begin(), dividendTimes_.end(), t);

        if (iter != dividendTimes_.end()) {
            const Real dividend = dividends_[iter - dividendTimes_.begin()];

            Array tmp(x_.size());
            Size xSpacing = mesher_->layout()->spacing()[equityDirection_];
            for (Size i=0; i<mesher_->layout()->dim().size(); ++i) {
                if (i!=equityDirection_) {
                    Size ySpacing = mesher_->layout()->spacing()[i];
                    for (Size j=0; j<mesher_->layout()->dim()[i]; ++j) {
                        for (Size k=0; k<x_.size(); ++k) {
                            Size index = j*ySpacing + k*xSpacing;
                            tmp[k] = aCopy[index];
                        }
                        LinearInterpolation interp(x_.begin(), x_.end(), 
                                                   tmp.begin());
                        for (Size k=0; k<x_.size(); ++k) {
                            Size index = j*ySpacing + k*xSpacing;
                            a[index] = interp(std::max(x_[0], x_[k]-dividend), 
                            		          true);
                        }
                    }
                }
            }
        }
    }
}
