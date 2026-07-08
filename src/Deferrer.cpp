#include "reactive/Computation.h"

#include <reactive/Deferrer.h>
#include <reactive/Deferrable.h>

#include <algorithm>
#include <queue>
#include <vector>

namespace Reactive
{
  thread_local Deferrer *tl_deferrer;

  Deferrer::Deferrer()
  {
    if(!tl_deferrer)
      tl_deferrer = this;
  }

  Deferrer::~Deferrer()
  {
    if(tl_deferrer != this)
      return;

    tl_deferrer = nullptr;

    auto cp = std::move(m_pending);

    // Process the pending deferrables strictly in ascending computation depth (parents
    // before children), one computation at a time, until nothing dirty remains - exactly
    // like the previous implementation. The previous code re-scanned ALL deferrables to
    // find the global minimum on every single step, which is O(pending^2) and made large
    // invalidation batches (e.g. rotating every step LED of every selected tile) freeze
    // the UI for seconds. Instead, we keep the deferrables in a depth-ordered priority
    // queue and only re-file the one we just touched.
    //
    // New invalidations triggered while re-running a computation are collected by the
    // nested Deferrer created around doDeferred() and flushed recursively, so they never
    // enter this queue - identical to before. getLowest() is therefore re-queried lazily
    // after each step to pick up any remaining or freshly-lowered pending work.
    struct Entry
    {
      uint32_t depth;
      size_t index;
    };

    auto deeper = [](const Entry &a, const Entry &b) { return a.depth > b.depth; };
    std::priority_queue<Entry, std::vector<Entry>, decltype(deeper)> queue(deeper);

    auto enqueueLowestOf = [&](size_t index)
    {
      if(auto s = cp[index].lock())
        if(auto c = s->getLowest(nullptr))
          queue.push({ c->getDepth(), index });
    };

    for(size_t i = 0; i < cp.size(); i++)
      enqueueLowestOf(i);

    while(!queue.empty())
    {
      auto entry = queue.top();
      queue.pop();

      auto s = cp[entry.index].lock();
      if(!s)
        continue;

      auto c = s->getLowest(nullptr);
      if(!c)
        continue;  // already drained in the meantime

      if(c->getDepth() != entry.depth)
      {
        // Its lowest pending depth changed (e.g. it was re-invalidated at a lower depth);
        // re-file it so it is still processed in correct global depth order.
        queue.push({ c->getDepth(), entry.index });
        continue;
      }

      {
        Deferrer deferrer;
        s->doDeferred(c);
      }

      enqueueLowestOf(entry.index);
    }
  }

  void Deferrer::add(const std::shared_ptr<Deferrable>& pending)
  {
    if(!tl_deferrer)
      pending->doDeferred(pending->getLowest(nullptr));
    else
      tl_deferrer->m_pending.push_back(pending);
  }

  void Deferrer::remove(const std::shared_ptr<Deferrable>& pending)
  {
    if(!tl_deferrer)
      return;

    auto& queue = tl_deferrer->m_pending;
    std::erase_if(
      queue,
      [&](const std::weak_ptr<Deferrable>& entry)
      {
        if(const auto locked = entry.lock())
          return locked == pending;

        return false;
      });
  }

  const std::vector<std::weak_ptr<Deferrable>> &Deferrer::getPending() const
  {
    return m_pending;
  }
}