#include "reactive/Computation.h"

#include <reactive/Deferrer.h>
#include <reactive/Deferrable.h>

#include <algorithm>
#include <chrono>
#include <iostream>

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

      while(true)
      {
        std::pair<Deferrable *, Computation *> lowestDepth = { nullptr, nullptr };

        for(auto &w : cp)
        {
          if(auto s = w.lock())
          {
            auto newLowestDepth = s->getLowest(lowestDepth.second);

            if(newLowestDepth != lowestDepth.second)
              lowestDepth = { s.get(), newLowestDepth };
          }
        }

        if(lowestDepth.first && lowestDepth.second)
        {
          Deferrer deferrer;
          lowestDepth.first->doDeferred(lowestDepth.second);
        }
        else
          break;
      }
    }
  }

  void Deferrer::add(std::shared_ptr<Deferrable> pending)
  {
    if(!tl_deferrer)
      pending->doDeferred(pending->getLowest(nullptr));
    else
      tl_deferrer->m_pending.push_back(pending);
  }
}