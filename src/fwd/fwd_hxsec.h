#pragma once

#include <memory>

#include "../species_tags.h"
#include "../xsec_fit.h"

namespace fwd::hxsec {
struct single {
  Numeric scl{};
  Numeric P{};
  Numeric T{};
  Verbosity verbosity{};
  std::shared_ptr<XsecRecord> xsecrec{};

  single() = default;
  
  single(Numeric p,
         Numeric t,
         Numeric VMR,
         const std::shared_ptr<XsecRecord>& cia,
         Verbosity verb = {});

  [[nodiscard]] Complex at(Numeric f) const;
  void at(ExhaustiveComplexVectorView abs, const Vector& fs) const;
  [[nodiscard]] ComplexVector at(const Vector& fs) const;
};

struct full {
  std::vector<single> models{};

  full() = default;

  full(Numeric p,
       Numeric t,
       const Vector& vmrs,
       const ArrayOfArrayOfSpeciesTag& allspecs,
       const std::vector<std::shared_ptr<XsecRecord>>& xsec,
       Verbosity verb = {});

  [[nodiscard]] Complex at(Numeric f) const;
  void at(ExhaustiveComplexVectorView abs, const Vector& fs) const;
  [[nodiscard]] ComplexVector at(const Vector& fs) const;
};
}  // namespace fwd::hxsec
