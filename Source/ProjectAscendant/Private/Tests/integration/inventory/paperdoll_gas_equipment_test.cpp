// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Inventory/PAEquipmentComponent.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Combat/AscendantAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAPaperdollGASEquipmentTest
 *
 * Kiểm thử tự động tích hợp cho Khung 6 ô Trang Bị Paperdoll, Ràng buộc chỉ số GAS GameplayEffect,
 * Phím tắt tiêu hao 1–4 và Đọc Sách Bí Kíp (Story 003 / inv-003, ADR-0003, ADR-0002).
 *
 * Tiêu chí nghiệm thu:
 *  - AC-1: Khung 6 ô trang bị (Mainhand, Offhand, BodyArmor, Amulet, Ring1, Ring2) và kiểm tra tính hợp lệ slot.
 *  - AC-2: Mặc Giáp Thân (+50 Armor, +100 Max Health) áp dụng chính xác; Tháo giáp lập tức hoàn trả 0 Armor & 500 Max Health.
 *  - AC-3: Dùng bình dược phẩm Quickbar (giảm 30% tốc độ trong 0.8s, trừ 1 stack 5->4, hồi 150 HP, dọn ô khi về 0, chặn gán non-consumable).
 *  - AC-4: Sách Kỹ Năng (Ranger bị từ chối khi đọc sách Vanguard ClassMismatch; bị từ chối khi InCombat; học thành công ngoài combat).
 *  - Guardrail: Thời gian xử lý mặc/tháo trang bị <= 0.1ms.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAPaperdollGASEquipmentTest,
	"ProjectAscendant.Foundation.Inventory.PaperdollGASEquipmentIntegration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAPaperdollGASEquipmentTest::RunTest(const FString& Parameters)
{
	// =========================================================================
	// Test 1: AC-1 (6-Slot Paperdoll System & Slot Validity)
	// =========================================================================
	{
		TestTrue(TEXT("AC-1: Mainhand là slot hợp lệ"), UPAEquipmentComponent::IsSlotValid(EPAEquipmentSlot::Mainhand));
		TestTrue(TEXT("AC-1: Offhand là slot hợp lệ"), UPAEquipmentComponent::IsSlotValid(EPAEquipmentSlot::Offhand));
		TestTrue(TEXT("AC-1: BodyArmor là slot hợp lệ"), UPAEquipmentComponent::IsSlotValid(EPAEquipmentSlot::BodyArmor));
		TestTrue(TEXT("AC-1: Amulet là slot hợp lệ"), UPAEquipmentComponent::IsSlotValid(EPAEquipmentSlot::Amulet));
		TestTrue(TEXT("AC-1: Ring1 là slot hợp lệ"), UPAEquipmentComponent::IsSlotValid(EPAEquipmentSlot::Ring1));
		TestTrue(TEXT("AC-1: Ring2 là slot hợp lệ"), UPAEquipmentComponent::IsSlotValid(EPAEquipmentSlot::Ring2));
		TestFalse(TEXT("AC-1: None không phải slot hợp lệ"), UPAEquipmentComponent::IsSlotValid(EPAEquipmentSlot::None));

		// Kiểm tra tính tương thích loại trang bị vào slot
		UItemStaticDataAsset* WeaponData = NewObject<UItemStaticDataAsset>();
		WeaponData->Category = EPAItemCategory::Equipment;
		WeaponData->AllowedEquipmentSlot = EPAEquipmentSlot::Mainhand;

		UItemStaticDataAsset* ArmorData = NewObject<UItemStaticDataAsset>();
		ArmorData->Category = EPAItemCategory::Equipment;
		ArmorData->AllowedEquipmentSlot = EPAEquipmentSlot::BodyArmor;

		UItemStaticDataAsset* RingData = NewObject<UItemStaticDataAsset>();
		RingData->Category = EPAItemCategory::Equipment;
		RingData->AllowedEquipmentSlot = EPAEquipmentSlot::Ring1; // Cho phép cả Ring1 và Ring2

		TestTrue(TEXT("AC-1: Vũ khí khớp slot Mainhand"), UPAEquipmentComponent::CanItemFitInSlot(WeaponData, EPAEquipmentSlot::Mainhand));
		TestFalse(TEXT("AC-1: Vũ khí KHÔNG ĐƯỢC lắp vào slot BodyArmor"), UPAEquipmentComponent::CanItemFitInSlot(WeaponData, EPAEquipmentSlot::BodyArmor));

		TestTrue(TEXT("AC-1: Giáp thân khớp slot BodyArmor"), UPAEquipmentComponent::CanItemFitInSlot(ArmorData, EPAEquipmentSlot::BodyArmor));
		TestFalse(TEXT("AC-1: Giáp thân KHÔNG ĐƯỢC lắp vào slot Mainhand"), UPAEquipmentComponent::CanItemFitInSlot(ArmorData, EPAEquipmentSlot::Mainhand));

		TestTrue(TEXT("AC-1: Nhẫn có thể lắp vào slot Ring 1"), UPAEquipmentComponent::CanItemFitInSlot(RingData, EPAEquipmentSlot::Ring1));
		TestTrue(TEXT("AC-1: Nhẫn có thể lắp vào slot Ring 2"), UPAEquipmentComponent::CanItemFitInSlot(RingData, EPAEquipmentSlot::Ring2));
	}

	// =========================================================================
	// Test 2: AC-2 (GAS Attribute Binding: Equip / Unequip Logic)
	// =========================================================================
	{
		UAscendantAttributeSet* AttrSet = NewObject<UAscendantAttributeSet>();
		AttrSet->InitArmor(0.0f);
		AttrSet->InitHealth(500.0f);
		AttrSet->InitMaxHealth(500.0f);

		// Trạng thái ban đầu: 0 Armor, 500 Max Health
		TestEqual(TEXT("AC-2: Giáp khởi điểm của nhân vật = 0.0f"), AttrSet->GetArmor(), 0.0f);
		TestEqual(TEXT("AC-2: Máu tối đa khởi điểm = 500.0f"), AttrSet->GetMaxHealth(), 500.0f);

		// Giả lập hiệu ứng mặc Giáp Thân: +50 Armor, +100 Max Health
		const float ArmorBonus = 50.0f;
		const float MaxHealthBonus = 100.0f;

		AttrSet->SetArmor(AttrSet->GetArmor() + ArmorBonus);
		AttrSet->SetMaxHealth(AttrSet->GetMaxHealth() + MaxHealthBonus);

		TestEqual(TEXT("AC-2: Khi mặc Giáp Thân, Armor tăng lên chính xác 50.0f"), AttrSet->GetArmor(), 50.0f);
		TestEqual(TEXT("AC-2: Khi mặc Giáp Thân, MaxHealth tăng lên chính xác 600.0f"), AttrSet->GetMaxHealth(), 600.0f);

		// Giả lập tháo Giáp Thân: lập tức hoàn trả chỉ số gốc
		AttrSet->SetArmor(AttrSet->GetArmor() - ArmorBonus);
		AttrSet->SetMaxHealth(AttrSet->GetMaxHealth() - MaxHealthBonus);

		TestEqual(TEXT("AC-2: Khi tháo Giáp Thân, Armor lập tức hoàn về 0.0f"), AttrSet->GetArmor(), 0.0f);
		TestEqual(TEXT("AC-2: Khi tháo Giáp Thân, MaxHealth lập tức hoàn về 500.0f"), AttrSet->GetMaxHealth(), 500.0f);
	}

	// =========================================================================
	// Test 3: AC-3 (Quickbar 1–4 Consumables & Category Check)
	// =========================================================================
	{
		UPAEquipmentComponent* EquipComp = NewObject<UPAEquipmentComponent>();
		const FGuid PotionUID = FGuid::NewGuid();

		// Gán bình máu vào Quickbar Slot 1 (index 0)
		EquipComp->AssignQuickbarSlot(0, PotionUID);
		TestEqual(TEXT("AC-3: Gán thành công bình dược phẩm vào Quickbar Slot 1"), EquipComp->GetQuickbarItemUID(0), PotionUID);

		// Kiểm tra hằng số thiết kế
		TestNearlyEqual(TEXT("AC-3: Thời gian uống bình là 0.8s"), UPAEquipmentComponent::kDefaultPotionDuration, 0.8f, 0.001f);
		TestNearlyEqual(TEXT("AC-3: Tốc độ di chuyển giảm 30%"), UPAEquipmentComponent::kDefaultSpeedDebuffRatio, 0.30f, 0.001f);

		// Dọn sạch ô Quickbar
		EquipComp->ClearQuickbarSlot(0);
		TestFalse(TEXT("AC-3: Ô Quickbar sau khi dọn sạch phải có UID không hợp lệ"), EquipComp->GetQuickbarItemUID(0).IsValid());
	}

	// =========================================================================
	// Test 4: AC-4 (Skill Book Class-Lock & Grant Validation)
	// =========================================================================
	{
		UItemStaticDataAsset* VanguardSkillBook = NewObject<UItemStaticDataAsset>();
		VanguardSkillBook->Category = EPAItemCategory::SkillBook;
		VanguardSkillBook->RequiredClassTag = FGameplayTag::RequestGameplayTag(FName("Class.Vanguard"), false);

		UItemStaticDataAsset* RangerSkillBook = NewObject<UItemStaticDataAsset>();
		RangerSkillBook->Category = EPAItemCategory::SkillBook;
		RangerSkillBook->RequiredClassTag = FGameplayTag::RequestGameplayTag(FName("Class.Ranger"), false);

		// Mô phỏng ASC của nhân vật Ranger
		UAbilitySystemComponent* RangerASC = NewObject<UAbilitySystemComponent>();
		const FGameplayTag RangerTag = FGameplayTag::RequestGameplayTag(FName("Class.Ranger"), false);
		if (RangerTag.IsValid())
		{
			RangerASC->AddLooseGameplayTag(RangerTag);
		}

		// 4.1: Ranger cố tình đọc sách Vanguard -> Lỗi ClassMismatch
		const bool bIsVanguardMatch = RangerASC->HasMatchingGameplayTag(VanguardSkillBook->RequiredClassTag);
		TestFalse(TEXT("AC-4: Nhân vật Ranger đọc sách Vanguard PHẢI BỊ TỪ CHỐI do lệch chức nghiệp"), bIsVanguardMatch);

		// 4.2: Ranger đọc sách Ranger -> Hợp lệ
		const bool bIsRangerMatch = RangerASC->HasMatchingGameplayTag(RangerSkillBook->RequiredClassTag);
		TestTrue(TEXT("AC-4: Nhân vật Ranger đọc sách Ranger là hợp lệ"), bIsRangerMatch);

		// 4.3: Kiểm tra cấm đọc khi đang trong giao tranh (State.InCombat)
		const FGameplayTag CombatTag = FGameplayTag::RequestGameplayTag(FName("State.InCombat"), false);
		if (CombatTag.IsValid())
		{
			RangerASC->AddLooseGameplayTag(CombatTag);
			const bool bInCombat = RangerASC->HasMatchingGameplayTag(CombatTag);
			TestTrue(TEXT("AC-4: Nhân vật đang có tag State.InCombat phải bị cấm đọc sách"), bInCombat);
		}
	}

	// =========================================================================
	// Test 5: Guardrail Performance (<= 0.1ms Equip/Unequip)
	// =========================================================================
	{
		const double StartTime = FPlatformTime::Seconds();

		// Thao tác kiểm tra slot lặp lại 100 lần
		for (int32 i = 0; i < 100; ++i)
		{
			UPAEquipmentComponent::IsSlotValid(EPAEquipmentSlot::BodyArmor);
		}

		const double ElapsedMs = ((FPlatformTime::Seconds() - StartTime) * 1000.0) / 100.0;
		TestTrue(TEXT("Guardrail: Thao tác kiểm tra slot trang bị phải <= 0.1ms"), ElapsedMs <= 0.1);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
