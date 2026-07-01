#pragma once

#include <reactive/Deferrable.h>
#include <reactive/Invalidateable.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include <utility>

namespace Reactive
{
  class Computation;

  class ComputationsImpl : public std::enable_shared_from_this<ComputationsImpl>, public Invalidateable, public Deferrable
  {
   public:
    ComputationsImpl();
    virtual ~ComputationsImpl();

    void add(std::function<void()> &&cb);
    void cancelPending();
    void invalidate(Computation *c) override;
    void resolveDirtynessDownstream() override;
    void doDeferred(Computation *c) override;
    Computation *getLowest(Computation *lowestSoFar) const override;

   private:
    std::unordered_map<Computation *, std::unique_ptr<Computation>> m_computations;
    std::unordered_map<Computation *, std::unique_ptr<Computation>> m_pending;
    std::set<std::pair<uint32_t, Computation *>> m_pendingOrder;
  };
}
