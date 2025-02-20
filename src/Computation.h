#pragma once

#include <unordered_set>
#include <functional>
#include "Invalidateable.h"

namespace Reactive
{
  class ComputationsImpl;
  class Invalidateable;

  namespace Detail
  {
    class VarBase;
  }

  class Computation
  {
   public:
    using Callback = std::function<void()>;

    static Computation *getCurrentComputation();

    Computation(Invalidateable &owner, Callback &&cb);
    ~Computation();

    void execute();
    void invalidate();

    Callback &&expropriateCallback();

    void registerVar(Detail::VarBase *v);
    void unregisterVar(Detail::VarBase *v);
    void resolveDirtynessDownstream();

   private:
    std::unordered_set<Detail::VarBase *> m_registeredVars;
    Invalidateable &m_owner;
    Callback m_cb;
  };

}
