#include <reactive/Deferrer.h>
#include <reactive/Computation.h>

#include "ComputationsImpl.h"

namespace Reactive
{
  ComputationsImpl::ComputationsImpl() = default;
  ComputationsImpl::~ComputationsImpl() = default;

  void ComputationsImpl::add(std::function<void()> &&cb)
  {
    auto computation = std::make_unique<Computation>(*this, std::move(cb));
    auto *raw = computation.get();
    m_computations.emplace(raw, std::move(computation));
    raw->execute();
  }

  void ComputationsImpl::clear()
  {
    // Called when the owning Computations dies. A Deferrer flush may still hold a
    // shared_ptr to this impl (it locks the weak_ptr while processing), keeping it
    // alive beyond its owner. Without retiring the computations here, the remainder
    // of that flush would re-run still-pending computations of the dead owner.
    m_pending.clear();
    m_computations.clear();
  }

  void ComputationsImpl::invalidate(Computation *c)
  {
    const bool wasEmpty = m_pending.empty();

    m_pending.insert({ c->getDepth(), c });

    if(wasEmpty && !m_pending.empty())
      Deferrer::add(shared_from_this());
  }

  void ComputationsImpl::resolveDirtynessDownstream()
  {
  }

  void ComputationsImpl::doDeferred(Computation *c)
  {
    m_pending.erase({ c->getDepth(), c });

    auto it = m_computations.find(c);

    if(it != m_computations.end())
    {
      auto cb = std::move(c->expropriateCallback());
      m_computations.erase(it);
      add(std::move(cb));
    }
  }

  Computation *ComputationsImpl::getLowest(Computation *lowestSoFar) const
  {
    if(m_pending.empty())
      return lowestSoFar;

    auto *my = m_pending.begin()->second;

    if(!lowestSoFar)
      return my;

    return my->getDepth() < lowestSoFar->getDepth() ? my : lowestSoFar;
  }
}
