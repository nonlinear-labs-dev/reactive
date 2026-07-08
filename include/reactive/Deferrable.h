#pragma once

namespace Reactive
{
  class Computation;

  class Deferrable
  {
   public:
    virtual ~Deferrable() = default;

    virtual Computation *getLowest(Computation *lowestSoFar) const = 0;
    virtual void doDeferred(Computation *c) = 0;
    virtual void clear() {}
  };
}
