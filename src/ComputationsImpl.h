#pragma once

#include <Deferrable.h>
#include <Invalidateable.h>

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
    void doDeferred() override;

   private:
    std::vector<std::unique_ptr<Computation>> m_computations;
    std::vector<std::function<void()>> m_pending;
  };
}