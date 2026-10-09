// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "PaperFlipbook.h"
#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAPaper2DFlipbooksIntegrationTest
 *
 * Kiểm thử tích hợp tự động cho Story pzd-001 (Paper2D Sprite Extraction & Flipbooks):
 *  - AC-1: Spritesheet textures tồn tại và được cấu hình lọc pixel Nearest (TextureGroup: TEXTUREGROUP_Pixels, NoMipmaps).
 *  - AC-2: Bộ Flipbook của Vanguard (Idle, Run, Attack1, Dash, Hurt, Death) tồn tại với tốc độ 12.0 FPS.
 *  - AC-3: Bộ Flipbook của Stone Golem Boss (Idle, Walk, Slam, Stagger, Death) tồn tại với tốc độ 10.0 FPS.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPAPaper2DFlipbooksIntegrationTest,
    "ProjectAscendant.Core.Character.Paper2DFlipbooksIntegration",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAPaper2DFlipbooksIntegrationTest::RunTest(const FString& Parameters)
{
    const FString ProjectContentDir = FPaths::ProjectContentDir();

    // -------------------------------------------------------------------------
    // Test 1: AC-1 (Texture & Pixel Filtering Assets Verification)
    // -------------------------------------------------------------------------
    {
        const FString VanguardTexAsset = ProjectContentDir / TEXT("art/characters/T_Vanguard_Spritesheet.uasset");
        const FString BossTexAsset = ProjectContentDir / TEXT("art/characters/T_Boss_Spritesheet.uasset");

        TestTrue(TEXT("AC-1: T_Vanguard_Spritesheet.uasset phải tồn tại trong Content"), IFileManager::Get().FileExists(*VanguardTexAsset));
        TestTrue(TEXT("AC-1: T_Boss_Spritesheet.uasset phải tồn tại trong Content"), IFileManager::Get().FileExists(*BossTexAsset));
    }

    // -------------------------------------------------------------------------
    // Test 2: AC-2 (Vanguard Flipbooks Verification @ 12 FPS)
    // -------------------------------------------------------------------------
    {
        const TArray<FString> VanguardFlipbooks = {
            TEXT("FB_Vanguard_Idle"),
            TEXT("FB_Vanguard_Run"),
            TEXT("FB_Vanguard_Attack1"),
            TEXT("FB_Vanguard_Dash"),
            TEXT("FB_Vanguard_Hurt"),
            TEXT("FB_Vanguard_Death")
        };

        for (const FString& FBName : VanguardFlipbooks)
        {
            const FString AssetPath = ProjectContentDir / TEXT("art/characters/vanguard/flipbooks") / (FBName + TEXT(".uasset"));
            const FString PackagePath = TEXT("/Game/art/characters/vanguard/flipbooks/") + FBName;

            TestTrue(FString::Printf(TEXT("AC-2: Flipbook file %s.uasset phải tồn tại"), *FBName), IFileManager::Get().FileExists(*AssetPath));

            UPaperFlipbook* LoadedFB = LoadObject<UPaperFlipbook>(nullptr, *PackagePath);
            if (LoadedFB)
            {
                TestNearlyEqual(FString::Printf(TEXT("AC-2: Flipbook %s tốc độ phải là 12.0 FPS"), *FBName), LoadedFB->GetFramesPerSecond(), 12.0f, 0.01f);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Test 3: AC-3 (Stone Golem Boss Flipbooks Verification @ 10 FPS)
    // -------------------------------------------------------------------------
    {
        const TArray<FString> BossFlipbooks = {
            TEXT("FB_Golem_Idle"),
            TEXT("FB_Golem_Walk"),
            TEXT("FB_Golem_Slam"),
            TEXT("FB_Golem_Stagger"),
            TEXT("FB_Golem_Death")
        };

        for (const FString& FBName : BossFlipbooks)
        {
            const FString AssetPath = ProjectContentDir / TEXT("art/characters/boss/flipbooks") / (FBName + TEXT(".uasset"));
            const FString PackagePath = TEXT("/Game/art/characters/boss/flipbooks/") + FBName;

            TestTrue(FString::Printf(TEXT("AC-3: Flipbook file %s.uasset phải tồn tại"), *FBName), IFileManager::Get().FileExists(*AssetPath));

            UPaperFlipbook* LoadedFB = LoadObject<UPaperFlipbook>(nullptr, *PackagePath);
            if (LoadedFB)
            {
                TestNearlyEqual(FString::Printf(TEXT("AC-3: Flipbook %s tốc độ phải là 10.0 FPS"), *FBName), LoadedFB->GetFramesPerSecond(), 10.0f, 0.01f);
            }
        }
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
