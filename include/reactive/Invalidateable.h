#pragma once

namespace Reactive
{
  class Computation;

  class Invalidateable
  {
   public:
    virtual ~Invalidateable() = default;
    virtual void invalidate(Computation* comp) = 0;
    virtual void resolveDirtynessDownstream() = 0;
  };
}