# Unreal Engine Automation Tests — Source Integration

**Project**: Project Ascendant  
**Target Engine**: Unreal Engine 5.8.2

---

## Overview

All automated test classes in C++ use the Unreal Engine Automation Testing Framework.
Only `.cpp` files inside the `ProjectAscendant` module (`Source/ProjectAscendant/`) are compiled,
so every automation test must live there. Tests are found in two places:

- `Source/ProjectAscendant/Private/<System>/PA*Tests.cpp` — tests that sit next to the system they cover.
- `Source/ProjectAscendant/Private/Tests/unit/<system>/` and `Source/ProjectAscendant/Private/Tests/integration/<system>/` —
  the unit/integration tests that used to live in the repo-root `Tests/` folder (moved in X12, 2026-10-09).

The repo-root `Tests/` folder is **not** compiled by any module; it only holds `smoke/` checklists,
`evidence/` and its README. Do not put `.cpp` test files there.

Every test must be wrapped in `#if WITH_DEV_AUTOMATION_TESTS` and use a name starting with
`ProjectAscendant.` so that `Automation RunTests ProjectAscendant.` (see `Tools/QA/run_headless_tests.sh --ue`) runs it.

## Class Architecture

- **Unit Tests**: Use `IMPLEMENT_SIMPLE_AUTOMATION_TEST(ClassName, TestPath, TestFlags)`
- **Complex Tests**: Use `IMPLEMENT_COMPLEX_AUTOMATION_TEST(ClassName, TestPath, TestFlags)` for data-driven test generation.

```cpp
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProjectAscendantSampleTest,
    "ProjectAscendant.Foundation.SampleVerification",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FProjectAscendantSampleTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Sample check"), true);
    return true;
}

#endif
```
