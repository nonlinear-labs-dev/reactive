#pragma once

namespace Reactive
{
  class Computation;
}
namespace Reactive
{
  class Deferrable
  {
   public:
    Deferrable() = default;

    virtual Computation *getLowest(Computation *lowestSoFar) const = 0;
    virtual void doDeferred(Computation *c) = 0;
  };
}