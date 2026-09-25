// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Character/PAPaperdollComponent.h"
#include "Character/PAPaperdollTypes.h"
#include "PaperFlipbookComponent.h"
#include "GameplayTagsManager.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPACharacterVisualIdentityTest
 *
 * Kiểm thử tự động cho Story visual-002 (EPIC-CHARACTER-VISUAL-001, Sprint 7):
 * - AC-1: Khởi tạo Socket_HelmCrest (64, 40) và Socket_Tabard (64, 60), kích thước 32x32 và 48x64.
 * - AC-2: Auto Class Identity Binding (12 class GameplayTags tự động gán Crest và Tabard, không bị ẩn khi tháo/mặc giáp).
 * - AC-3: Directional Sort Key (Tabard 26 > Chest 25, HelmCrest 36 > Helm 35, bảo toàn khi mirror hướng Tây).
 * - AC-4: Ma trận 3 hạng cân giáp (Heavy/Medium/Light) x 12 Class tags, 100% Headless Safe, Không rò rỉ hay crash null pointer.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPACharacterVisualIdentityTest,
	"ProjectAscendant.CharacterVisual.IdentitySocketsAndOverlays",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPACharacterVisualIdentityTest::RunTest(const FString& Parameters)
{
	// =========================================================================
	// AC-1: Khởi Tạo Sockets & Sub-Components Mới
	// =========================================================================
	{
		TestEqual(TEXT("AC-1: Socket_HelmCrest name"),
			FPAPaperdollConstants::Socket_HelmCrest, FName(TEXT("Socket_HelmCrest")));
		TestEqual(TEXT("AC-1: Socket_Tabard name"),
			FPAPaperdollConstants::Socket_Tabard, FName(TEXT("Socket_Tabard")));

		TestEqual(TEXT("AC-1: HelmCrestLocation is strictly (64, 40)"),
			FPAPaperdollConstants::HelmCrestLocation, FVector2D(64.0f, 40.0f));
		TestEqual(TEXT("AC-1: TabardLocation is strictly (64, 60)"),
			FPAPaperdollConstants::TabardLocation, FVector2D(64.0f, 60.0f));

		TestEqual(TEXT("AC-1: Crest dimensions are strictly 32x32"),
			FPAPaperdollConstants::CrestPlaceholderDimensions, FIntPoint(32, 32));
		TestEqual(TEXT("AC-1: Tabard dimensions are strictly 48x64"),
			FPAPaperdollConstants::TabardPlaceholderDimensions, FIntPoint(48, 64));

		// Test khởi tạo sub-components qua UPAPaperdollComponent
		UPAPaperdollComponent* Comp = NewObject<UPAPaperdollComponent>();
		TestNotNull(TEXT("AC-1: UPAPaperdollComponent instantiated"), Comp);

		UPaperFlipbookComponent* CrestComp = NewObject<UPaperFlipbookComponent>();
		UPaperFlipbookComponent* TabardComp = NewObject<UPaperFlipbookComponent>();

		Comp->RegisterHelmCrestComponent(CrestComp);
		Comp->RegisterTabardComponent(TabardComp);

		TestEqual(TEXT("AC-1: HelmCrestComponent registered"), Comp->GetHelmCrestComponent(), CrestComp);
		TestEqual(TEXT("AC-1: TabardComponent registered"), Comp->GetTabardComponent(), TabardComp);
		TestEqual(TEXT("AC-1: HelmCrest initial sort priority is 36"), CrestComp->TranslucencySortPriority, 36);
		TestEqual(TEXT("AC-1: Tabard initial sort priority is 26"), TabardComp->TranslucencySortPriority, 26);
	}

	// =========================================================================
	// AC-2: Auto Class Identity Binding & Armor Decoupling
	// =========================================================================
	{
		struct FClassSpec
		{
			FName ClassName;
			FName ExpectedCrestAsset;
			FName ExpectedTabardAsset;
		};

		const FClassSpec AllClasses[] = {
			{ FName(TEXT("Vanguard")),     FName(TEXT("FB_Crest_Vanguard")),     FName(TEXT("FB_Tabard_Vanguard")) },
			{ FName(TEXT("Ranger")),       FName(TEXT("FB_Crest_Ranger")),       FName(TEXT("FB_Tabard_Ranger")) },
			{ FName(TEXT("Arcanist")),     FName(TEXT("FB_Crest_Arcanist")),     FName(TEXT("FB_Tabard_Arcanist")) },
			{ FName(TEXT("Acolyte")),      FName(TEXT("FB_Crest_Acolyte")),      FName(TEXT("FB_Tabard_Acolyte")) },
			{ FName(TEXT("Berserker")),    FName(TEXT("FB_Crest_Berserker")),    FName(TEXT("FB_Tabard_Berserker")) },
			{ FName(TEXT("Shadowblade")),  FName(TEXT("FB_Crest_Shadowblade")),  FName(TEXT("FB_Tabard_Shadowblade")) },
			{ FName(TEXT("Elementalist")), FName(TEXT("FB_Crest_Elementalist")), FName(TEXT("FB_Tabard_Elementalist")) },
			{ FName(TEXT("Templar")),      FName(TEXT("FB_Crest_Templar")),      FName(TEXT("FB_Tabard_Templar")) },
			{ FName(TEXT("VoidBlade")),    FName(TEXT("FB_Crest_VoidBlade")),    FName(TEXT("FB_Tabard_VoidBlade")) },
			{ FName(TEXT("Chronomancer")), FName(TEXT("FB_Crest_Chronomancer")), FName(TEXT("FB_Tabard_Chronomancer")) },
			{ FName(TEXT("DragonKnight")), FName(TEXT("FB_Crest_DragonKnight")), FName(TEXT("FB_Tabard_DragonKnight")) },
			{ FName(TEXT("GodSlayer")),    FName(TEXT("FB_Crest_GodSlayer")),    FName(TEXT("FB_Tabard_GodSlayer")) },
		};

		FPAPaperdollModel Model;

		for (const FClassSpec& Spec : AllClasses)
		{
			// Test binding by Class Name
			Model.SetClassIdentity(Spec.ClassName);
			TestEqual(FString::Printf(TEXT("AC-2: [%s] Crest matches by name"), *Spec.ClassName.ToString()),
				Model.GetHelmCrestVisualAssetId(), Spec.ExpectedCrestAsset);
			TestEqual(FString::Printf(TEXT("AC-2: [%s] Tabard matches by name"), *Spec.ClassName.ToString()),
				Model.GetTabardVisualAssetId(), Spec.ExpectedTabardAsset);

			// Test binding by Gameplay Tag
			const FGameplayTag Tag = FPAPaperdollConstants::GetTagForClass(Spec.ClassName);
			TestTrue(FString::Printf(TEXT("AC-2: [%s] GameplayTag is valid"), *Spec.ClassName.ToString()), Tag.IsValid());

			const bool bTagSet = Model.SetClassIdentityByTag(Tag);
			TestTrue(FString::Printf(TEXT("AC-2: [%s] SetClassIdentityByTag succeeded"), *Spec.ClassName.ToString()), bTagSet);
			TestEqual(FString::Printf(TEXT("AC-2: [%s] Crest matches by tag"), *Spec.ClassName.ToString()),
				Model.GetHelmCrestVisualAssetId(), Spec.ExpectedCrestAsset);
			TestEqual(FString::Printf(TEXT("AC-2: [%s] Tabard matches by tag"), *Spec.ClassName.ToString()),
				Model.GetTabardVisualAssetId(), Spec.ExpectedTabardAsset);
		}

		// Test rằng tháo/mặc giáp Helm và Chest KHÔNG làm ẩn Crest và Tabard
		UPAPaperdollComponent* Comp = NewObject<UPAPaperdollComponent>();
		UPaperFlipbookComponent* CrestComp = NewObject<UPaperFlipbookComponent>();
		UPaperFlipbookComponent* TabardComp = NewObject<UPaperFlipbookComponent>();
		Comp->RegisterHelmCrestComponent(CrestComp);
		Comp->RegisterTabardComponent(TabardComp);

		Comp->SetClassIdentity(FName(TEXT("Templar")));
		TestEqual(TEXT("AC-2: Initial Templar Crest"), Comp->GetActiveHelmCrestVisualAssetId(), FName(TEXT("FB_Crest_Templar")));
		TestEqual(TEXT("AC-2: Initial Templar Tabard"), Comp->GetActiveTabardVisualAssetId(), FName(TEXT("FB_Tabard_Templar")));
		TestTrue(TEXT("AC-2: CrestComponent is visible"), CrestComp->IsVisible());
		TestTrue(TEXT("AC-2: TabardComponent is visible"), TabardComp->IsVisible());

		// Mặc mũ giáp sắt và áo giáp sắt hạng nặng
		Comp->EquipSlot(EPAPaperdollSlot::Helm, FName(TEXT("Armor_Heavy_Helm_T1")), FName(TEXT("Visual_HeavyHelm_T1")));
		Comp->EquipSlot(EPAPaperdollSlot::Chest, FName(TEXT("Armor_Heavy_Chest_T1")), FName(TEXT("Visual_HeavyChest_T1")));

		TestEqual(TEXT("AC-2: Helm slot equipped"), Comp->GetModel().GetActiveVisualAssetForSlot(EPAPaperdollSlot::Helm), FName(TEXT("Visual_HeavyHelm_T1")));
		TestEqual(TEXT("AC-2: Chest slot equipped"), Comp->GetModel().GetActiveVisualAssetForSlot(EPAPaperdollSlot::Chest), FName(TEXT("Visual_HeavyChest_T1")));
		// Crest và Tabard vẫn hiển thị đè lên trên!
		TestEqual(TEXT("AC-2: Crest remains visible when Helm equipped"), Comp->GetActiveHelmCrestVisualAssetId(), FName(TEXT("FB_Crest_Templar")));
		TestEqual(TEXT("AC-2: Tabard remains visible when Chest equipped"), Comp->GetActiveTabardVisualAssetId(), FName(TEXT("FB_Tabard_Templar")));
		TestTrue(TEXT("AC-2: CrestComp still visible"), CrestComp->IsVisible());
		TestTrue(TEXT("AC-2: TabardComp still visible"), TabardComp->IsVisible());

		// Tháo giáp Helm và Chest
		Comp->UnequipSlot(EPAPaperdollSlot::Helm);
		Comp->UnequipSlot(EPAPaperdollSlot::Chest);

		TestTrue(TEXT("AC-2: Helm unequipped"), Comp->GetModel().GetActiveVisualAssetForSlot(EPAPaperdollSlot::Helm).IsNone());
		TestTrue(TEXT("AC-2: Chest unequipped"), Comp->GetModel().GetActiveVisualAssetForSlot(EPAPaperdollSlot::Chest).IsNone());
		TestEqual(TEXT("AC-2: Crest intact after unequip"), Comp->GetActiveHelmCrestVisualAssetId(), FName(TEXT("FB_Crest_Templar")));
		TestEqual(TEXT("AC-2: Tabard intact after unequip"), Comp->GetActiveTabardVisualAssetId(), FName(TEXT("FB_Tabard_Templar")));
		TestTrue(TEXT("AC-2: CrestComp still visible after unequip"), CrestComp->IsVisible());
		TestTrue(TEXT("AC-2: TabardComp still visible after unequip"), TabardComp->IsVisible());
	}

	// =========================================================================
	// AC-3: Directional Sort Key & Mirroring
	// =========================================================================
	{
		UPAPaperdollComponent* Comp = NewObject<UPAPaperdollComponent>();

		UPaperFlipbookComponent* LowerComp = NewObject<UPaperFlipbookComponent>();
		UPaperFlipbookComponent* UpperComp = NewObject<UPaperFlipbookComponent>();
		UPaperFlipbookComponent* ChestSlotComp = NewObject<UPaperFlipbookComponent>();
		UPaperFlipbookComponent* HelmSlotComp = NewObject<UPaperFlipbookComponent>();
		UPaperFlipbookComponent* TabardComp = NewObject<UPaperFlipbookComponent>();
		UPaperFlipbookComponent* CrestComp = NewObject<UPaperFlipbookComponent>();

		Comp->RegisterLowerBodyComponent(LowerComp);
		Comp->RegisterUpperBodyComponent(UpperComp);
		Comp->RegisterSlotFlipbookComponent(EPAPaperdollSlot::Chest, ChestSlotComp);
		Comp->RegisterSlotFlipbookComponent(EPAPaperdollSlot::Helm, HelmSlotComp);
		Comp->RegisterTabardComponent(TabardComp);
		Comp->RegisterHelmCrestComponent(CrestComp);

		// 1. Kiểm tra khi ngắm hướng Đông (East - Unmirrored)
		Comp->UpdateDirectionalSortKeys(EPAAimDirection8Way::East);

		TestEqual(TEXT("AC-3: East LowerBody is 10"), LowerComp->TranslucencySortPriority, 10);
		TestEqual(TEXT("AC-3: East UpperBody is 20"), UpperComp->TranslucencySortPriority, 20);
		TestEqual(TEXT("AC-3: East Chest is 25"), ChestSlotComp->TranslucencySortPriority, 25);
		TestEqual(TEXT("AC-3: East Tabard is 26"), TabardComp->TranslucencySortPriority, 26);
		TestEqual(TEXT("AC-3: East Helm is 35"), HelmSlotComp->TranslucencySortPriority, 35);
		TestEqual(TEXT("AC-3: East HelmCrest is 36"), CrestComp->TranslucencySortPriority, 36);

		TestTrue(TEXT("AC-3: Tabard (26) strictly above Chest (25) in East"),
			TabardComp->TranslucencySortPriority > ChestSlotComp->TranslucencySortPriority);
		TestTrue(TEXT("AC-3: HelmCrest (36) strictly above Helm (35) in East"),
			CrestComp->TranslucencySortPriority > HelmSlotComp->TranslucencySortPriority);

		// 2. Kiểm tra khi ngắm hướng Tây (West - Mirrored)
		Comp->UpdateDirectionalSortKeys(EPAAimDirection8Way::West);

		TestEqual(TEXT("AC-3: West LowerBody stays 10"), LowerComp->TranslucencySortPriority, 10);
		TestEqual(TEXT("AC-3: West UpperBody stays 20"), UpperComp->TranslucencySortPriority, 20);
		TestEqual(TEXT("AC-3: West Chest stays 25"), ChestSlotComp->TranslucencySortPriority, 25);
		TestEqual(TEXT("AC-3: West Tabard stays 26"), TabardComp->TranslucencySortPriority, 26);
		TestEqual(TEXT("AC-3: West Helm stays 35"), HelmSlotComp->TranslucencySortPriority, 35);
		TestEqual(TEXT("AC-3: West HelmCrest stays 36"), CrestComp->TranslucencySortPriority, 36);

		// Xác nhận Tabard KHÔNG bị Chest đè bẹp khi mirror hướng Tây!
		TestTrue(TEXT("AC-3: Tabard (26) strictly above Chest (25) when mirrored West"),
			TabardComp->TranslucencySortPriority > ChestSlotComp->TranslucencySortPriority);
		TestTrue(TEXT("AC-3: HelmCrest (36) strictly above Helm (35) when mirrored West"),
			CrestComp->TranslucencySortPriority > HelmSlotComp->TranslucencySortPriority);

		// Kiểm tra FPAPaperdollSortKey::GetSortPriorityForIdentitySocket
		TestEqual(TEXT("AC-3: SortPriorityForIdentitySocket HelmCrest is 36"),
			FPAPaperdollSortKey::GetSortPriorityForIdentitySocket(FPAPaperdollConstants::Socket_HelmCrest, EPAAimDirection8Way::West), 36);
		TestEqual(TEXT("AC-3: SortPriorityForIdentitySocket Tabard is 26"),
			FPAPaperdollSortKey::GetSortPriorityForIdentitySocket(FPAPaperdollConstants::Socket_Tabard, EPAAimDirection8Way::West), 26);
	}

	// =========================================================================
	// AC-4: 3 Armor Weight Classes x 12 Classes Headless Stability Matrix
	// =========================================================================
	{
		UPAPaperdollComponent* Comp = NewObject<UPAPaperdollComponent>();
		UPaperFlipbookComponent* CrestComp = NewObject<UPaperFlipbookComponent>();
		UPaperFlipbookComponent* TabardComp = NewObject<UPaperFlipbookComponent>();
		Comp->RegisterHelmCrestComponent(CrestComp);
		Comp->RegisterTabardComponent(TabardComp);

		const FName ArmorWeightTiers[] = {
			FName(TEXT("Armor_Heavy_T1")),
			FName(TEXT("Armor_Medium_T1")),
			FName(TEXT("Armor_Light_T1"))
		};

		const FName ClassTags[] = {
			FName(TEXT("Class.Vanguard")),
			FName(TEXT("Class.Ranger")),
			FName(TEXT("Class.Arcanist")),
			FName(TEXT("Class.Acolyte")),
			FName(TEXT("Class.Berserker")),
			FName(TEXT("Class.Shadowblade")),
			FName(TEXT("Class.Elementalist")),
			FName(TEXT("Class.Templar")),
			FName(TEXT("Class.VoidBlade")),
			FName(TEXT("Class.Chronomancer")),
			FName(TEXT("Class.DragonKnight")),
			FName(TEXT("Class.GodSlayer"))
		};

		int32 MatrixEvaluations = 0;

		for (const FName& ArmorTier : ArmorWeightTiers)
		{
			// Mặc cả giáp ngực và mũ của hạng cân
			const FName ChestItemId = ArmorTier;
			const FName HelmItemId = FName(*FString::Printf(TEXT("%s_Helm"), *ArmorTier.ToString()));
			const FName ChestVisualId = FName(*FString::Printf(TEXT("Visual_%s"), *ArmorTier.ToString()));
			const FName HelmVisualId = FName(*FString::Printf(TEXT("Visual_%s_Helm"), *ArmorTier.ToString()));

			Comp->EquipSlot(EPAPaperdollSlot::Chest, ChestItemId, ChestVisualId);
			Comp->EquipSlot(EPAPaperdollSlot::Helm, HelmItemId, HelmVisualId);

			for (const FName& ClassTag : ClassTags)
			{
				const FGameplayTag Tag = UGameplayTagsManager::Get().AddNativeGameplayTag(ClassTag);
				const bool bSuccess = Comp->SetClassIdentityByTag(Tag);
				TestTrue(FString::Printf(TEXT("AC-4: SetClassIdentityByTag [%s] under [%s]"), *ClassTag.ToString(), *ArmorTier.ToString()), bSuccess);

				// Xác nhận Crest và Tabard luôn hợp lệ và không rỗng
				const FName ActiveCrest = Comp->GetActiveHelmCrestVisualAssetId();
				const FName ActiveTabard = Comp->GetActiveTabardVisualAssetId();

				TestFalse(FString::Printf(TEXT("AC-4: Crest valid for [%s] with [%s]"), *ClassTag.ToString(), *ArmorTier.ToString()), ActiveCrest.IsNone());
				TestFalse(FString::Printf(TEXT("AC-4: Tabard valid for [%s] with [%s]"), *ClassTag.ToString(), *ArmorTier.ToString()), ActiveTabard.IsNone());

				TestTrue(TEXT("AC-4: CrestComp is visible"), CrestComp->IsVisible());
				TestTrue(TEXT("AC-4: TabardComp is visible"), TabardComp->IsVisible());

				MatrixEvaluations++;
			}
		}

		TestEqual(TEXT("AC-4: Exactly 36 Armor x Class combinations evaluated with zero crash"), MatrixEvaluations, 36);

		// Kiểm tra tính năng ẩn/hiện thủ công (Visibility toggling)
		Comp->SetHelmCrestVisibility(false);
		TestTrue(TEXT("AC-4: HelmCrest hidden when set invisible"), Comp->GetActiveHelmCrestVisualAssetId().IsNone());
		TestFalse(TEXT("AC-4: CrestComp not visible"), CrestComp->IsVisible());

		Comp->SetHelmCrestVisibility(true);
		TestFalse(TEXT("AC-4: HelmCrest visible again"), Comp->GetActiveHelmCrestVisualAssetId().IsNone());
		TestTrue(TEXT("AC-4: CrestComp visible again"), CrestComp->IsVisible());

		Comp->SetTabardVisibility(false);
		TestTrue(TEXT("AC-4: Tabard hidden when set invisible"), Comp->GetActiveTabardVisualAssetId().IsNone());
		TestFalse(TEXT("AC-4: TabardComp not visible"), TabardComp->IsVisible());

		Comp->SetTabardVisibility(true);
		TestFalse(TEXT("AC-4: Tabard visible again"), Comp->GetActiveTabardVisualAssetId().IsNone());
		TestTrue(TEXT("AC-4: TabardComp visible again"), TabardComp->IsVisible());
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
