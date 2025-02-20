#pragma once

#include <reactive/Deferrable.h>
#include <reactive/Invalidateable.h>

#include <functional>
#include <memory>
#include <vector>

namespace Reactive
{
  class Computation;

  class ComputationsImpl : public std::enable_shared_from_this<ComputationsImpl>, public Invalidateable, public Deferrable
  {
   public:
    ComputationsImpl();
    virtual ~ComputationsImpl();

    void add(std::function<void()> &&cb);
    void invalidate(Computation *c) override;
    void resolveDirtynessDownstream() override;
    void doDeferred(Computation *c) override;
    Computation *getLowest(Computation *lowestSoFar) const override;

   private:
    std::vector<std::unique_ptr<Computation>> m_computations;
    std::vector<std::unique_ptr<Computation>> m_pending;
  };
}