#include <catch2/catch_all.hpp>
#include <reactive/Computations.h>
#include <reactive/Deferrer.h>
#include <reactive/Var.h>

#include <memory>

// A Deferrer flush locks the ComputationsImpl it is processing, so the impl can outlive
// its owning Computations object. The contract these tests pin down: once a Computations
// object is destroyed, none of its computations may ever run again - even if they are
// still pending in the very flush that destroys their owner. Violating this re-runs
// autoruns of dead widgets against freed captures (the level-meter teardown segfault).

using namespace Reactive;

TEST_CASE("computations of an owner destroyed mid-flush never run again")
{
  Var<int> trigger { 0 };
  Var<int> rebuildRequest { 0 };

  struct Child
  {
    Computations comps;
  };

  std::unique_ptr<Child> child;
  int aRuns = 0;
  int bRuns = 0;

  Computations parent;
  parent.add(
      [&]
      {
        if(rebuildRequest.get() == 0)
        {
          // Build the child from within the parent's autorun, exactly like widgets are
          // built: its computations have a greater depth than the parent's.
          child = std::make_unique<Child>();

          child->comps.add(
              [&]
              {
                aRuns++;
                if(trigger.get() == 1)
                  rebuildRequest = 1;
              });

          child->comps.add(
              [&]
              {
                bRuns++;
                (void) rebuildRequest.get();
              });
        }
        else
        {
          child.reset();
        }
      });

  REQUIRE(aRuns == 1);
  REQUIRE(bRuns == 1);

  {
    Deferrer d;
    trigger = 1;
  }

  // The flush re-ran A while holding the child's impl. A's write invalidated the parent
  // (depth 1) and B (depth 2); the nested flush processed the parent first, which
  // destroyed the child. B - pending, but owned by the dead child - must not have run.
  CHECK(aRuns == 2);
  CHECK(bRuns == 1);
}

TEST_CASE("pending computations die with their owner outside a flush")
{
  Var<int> trigger { 0 };

  struct Child
  {
    Computations comps;
  };

  int runs = 0;
  auto child = std::make_unique<Child>();
  child->comps.add(
      [&]
      {
        (void) trigger.get();
        runs++;
      });

  REQUIRE(runs == 1);

  {
    Deferrer d;
    trigger = 1;   // the child's computation is now pending
    child.reset(); // owner dies before the flush processes it
  }

  CHECK(runs == 1);
}
