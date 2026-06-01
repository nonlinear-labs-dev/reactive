#pragma once

#include <cstdint>
#include <functional>
#include <unordered_set>

namespace Reactive
{
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

    static void untracked(Callback &&cb);

    void execute();
    void invalidate();

    Callback &&expropriateCallback();

    void registerVar(Detail::VarBase *v);
    void unregisterVar(Detail::VarBase *v);
    void resolveDirtynessDownstream() const;

    uint32_t getDepth() const;

   private:
    std::unordered_set<Detail::VarBase *> m_registeredVars;
    Invalidateable &m_owner;
    Callback m_cb;
    uint32_t m_depth;
  };

}
