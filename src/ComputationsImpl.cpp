#include <reactive/Deferrer.h>

#include "ComputationsImpl.h"
#include "Computation.h"

#include <algorithm>

namespace Reactive
{
  ComputationsImpl::ComputationsImpl() = default;
  ComputationsImpl::~ComputationsImpl() = default;

  void ComputationsImpl::add(std::function<void()> &&cb)
  {
    m_computations.push_back(std::make_unique<Computation>(*this, std::move(cb)));
    m_computations.back()->execute();
  }

  void ComputationsImpl::invalidate(Computation *c)
  {
    m_pending.push_back(c->expropriateCallback());

    auto it = std::find_if(m_computations.begin(), m_computations.end(), [&](auto &m) { return m.get() == c; });

    if(it != m_computations.end())
      m_computations.erase(it);

    if(m_pending.size() == 1)
      Deferrer::add(shared_from_this());
  }

  void ComputationsImpl::resolveDirtynessDownstream()
  {
  }

  void ComputationsImpl::doDeferred()
  {
    std::vector<std::function<void()>> c;
    std::swap(c, m_pending);

    for(auto &&k : c)
      add(std::move(k));
  }
}
