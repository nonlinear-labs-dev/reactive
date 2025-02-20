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

      auto cp = std::move(m_pending);

      for(auto &w : cp)
        if(auto s = w.lock())
          s->doDeferred();
    }
  }

  void Deferrer::add(std::shared_ptr<Deferrable> pending)
  {
    if(!tl_deferrer)
      pending->doDeferred();
    else
      tl_deferrer->m_pending.push_back(pending);
  }
}