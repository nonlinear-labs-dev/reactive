#include "reactive/Computation.h"

#include <reactive/Deferrer.h>
#include <reactive/Deferrable.h>

#include <algorithm>
#include <chrono>

namespace Reactive
{
  using namespace std::chrono_literals;
  thread_local Deferrer *tl_deferrer;

  Deferrer::Deferrer()
  {
    if(!tl_deferrer)
      tl_deferrer = this;
  }

  Deferrer::~Deferrer()
  {
    if(tl_deferrer == this)
    {
      tl_deferrer = nullptr;
      while(!m_pending.empty())
      {
        auto [depth, weak_ptr] = m_pending.top();
        m_pending.pop();

        if(const auto s = weak_ptr.lock())
        {
          s->doDeferred(s->getLowest(nullptr));
        }
      }
    }
  }

  void Deferrer::add(const std::shared_ptr<Deferrable> &pending)
  {
    if(!tl_deferrer)
      pending->doDeferred(pending->getLowest(nullptr));
    else
      tl_deferrer->m_pending.push(tPendingEntry { pending->getLowest(nullptr)->getDepth(), pending });
  }
}