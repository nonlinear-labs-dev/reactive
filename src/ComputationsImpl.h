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
    void invalidate(Computation *c) override;
    void resolveDirtynessDownstream() override;
    void doDeferred(Computation *c) override;
    Computation *getLowest(Computation *lowestSoFar) const override;

   private:
    // Ownership of all live computations, keyed by pointer for O(1) lookup. The previous
    // implementation moved unique_ptrs between two vectors and located them with a linear
    // std::find_if on every invalidate/doDeferred - O(n) per call, O(n^2) when a whole
    // batch (e.g. all step LEDs) is invalidated at once. Here ownership is stable and the
    // dirty set is kept ordered by (depth, pointer) so the lowest-depth pending computation
    // is the first element.
    std::unordered_map<Computation *, std::unique_ptr<Computation>> m_computations;
    std::set<std::pair<uint32_t, Computation *>> m_pending;
  };
}