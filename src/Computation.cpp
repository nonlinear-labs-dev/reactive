#include <reactive/Var.h>
#include <reactive/Computations.h>
#include <reactive/Computation.h>

#include "ComputationsImpl.h"
#include "reactive/Deferrer.h"

#include <cassert>

namespace Reactive
{
  thread_local Computation *tl_currentComputation;

  Computation::Computation(Invalidateable &owner, Callback &&cb)
      : m_owner(owner)
      , m_cb(std::move(cb))
      , m_depth(getCurrentComputation() ? getCurrentComputation()->m_depth + 1 : 1)
  {
  }

  Computation::~Computation()
  {
    assert(tl_currentComputation != this);

    for(const auto v : m_registeredVars)
      v->unregisterComputation(this);
  }

  void Computation::execute()
  {
    auto pThis = this;
    std::swap(tl_currentComputation, pThis);
    try
    {
      m_cb();
    }
    catch(...)
    {
    }
    std::swap(tl_currentComputation, pThis);
  }

  void Computation::invalidate()
  {
    m_owner.invalidate(this);
  }

  Computation *Computation::getCurrentComputation()
  {
    return tl_currentComputation;
  }

  Computation::Callback &&Computation::expropriateCallback()
  {
    return std::move(m_cb);
  }

  void Computation::registerVar(Detail::VarBase *v)
  {
    m_registeredVars.insert(v);
  }

  void Computation::unregisterVar(Detail::VarBase *v)
  {
    m_registeredVars.erase(v);
    invalidate();
  }

  void Computation::resolveDirtynessDownstream()
  {
    Deferrer deferrer;
    for(const auto v : m_registeredVars)
      v->resolveDirtynessDownstream();
  }

  uint32_t Computation::getDepth() const
  {
    return m_depth;
  }
}
