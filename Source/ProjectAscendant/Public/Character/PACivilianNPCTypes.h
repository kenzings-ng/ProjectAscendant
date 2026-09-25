// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/PAPaperdollTypes.h"
#include "World/PAZoneTypes.h"
#include "PACivilianNPCTypes.generated.h"

/**
 * EPACivilianRole
 *
 * 5 Vai trò NPC Dân sự trong khu vực an toàn (Story visual-003, GDD §6.2):
 * - Blacksmith: Thợ rèn (Rig 1: HeavyTank, Búa & Kẹp rèn, liên kết UPABlacksmithSubsystem)
 * - Merchant: Thương nhân (Rig 2: Agility, Túi vàng & Cân tiểu ly, mua bán dược phẩm/quặng)
 * - Villager: Dân làng nền (Rig 2: Agility / Rig 3: Caster, Giỏ bánh & Chổi quét)
 * - TownGuard: Lính gác thị trấn (Rig 1: HeavyTank, Giáo & Khiên thành, AI Dual-State)
 * - QuestGiver: Người giao nhiệm vụ (Rig 3: Caster, Cuộn thư cổ, Animation Callout vẫy tay & !)
 */
UENUM(BlueprintType)
enum class EPACivilianRole : uint8
{
	Blacksmith    = 0 UMETA(DisplayName = "Thợ Rèn (Blacksmith)"),
	Merchant      = 1 UMETA(DisplayName = "Thương Nhân (Merchant)"),
	Villager      = 2 UMETA(DisplayName = "Dân Làng Nền (Villager / Ambient)"),
	TownGuard     = 3 UMETA(DisplayName = "Lính Gác Thị Trấn (Town Guard)"),
	QuestGiver    = 4 UMETA(DisplayName = "Người Giao Nhiệm Vụ (Quest Giver)")
};

/**
 * EPAGuardAIState
 *
 * Trạng thái AI 2 pha (Dual-State AI) của Lính Gác Thị Trấn:
 * - Passive: Đứng nghiêm hoặc tuần tra an toàn (Idle / Walk)
 * - CombatActive: Kích hoạt đòn đâm chí mạng Knockback khi phát hiện Outlaw
 */
UENUM(BlueprintType)
enum class EPAGuardAIState : uint8
{
	Passive       = 0 UMETA(DisplayName = "Thụ Động Tuần Tra (Passive Patrol / Guard)"),
	CombatActive  = 1 UMETA(DisplayName = "Chiến Đấu Trừng Phạt (Lethal Gatekeeper Strike)")
};

/**
 * FPACivilianPalettePreset
 *
 * Cấu hình Palette Swap đổi màu vải trang phục theo 3 Phân Vùng Thế Giới (GDD §6.3):
 * - Verdant Bastion: Nâu da bò (#8D6E63), Xanh rêu (#4E6E58), Vải lanh mộc (#D7CCC8)
 * - Ashen Keep: Xám tro (#424242), Đỏ gạch nung (#8D2B2B), Thép rỉ sét (#78909C)
 * - Sanctum Fortress: Trắng ngà (#F5F5F5), Lam ngọc thẫm (#1A237E), Chỉ vàng kim (#FFD54F)
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACivilianPalettePreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Palette")
	FName ZoneId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Palette")
	FLinearColor PrimaryColor = FLinearColor(0.553f, 0.431f, 0.388f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Palette")
	FLinearColor SecondaryColor = FLinearColor(0.306f, 0.431f, 0.345f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Palette")
	FLinearColor AccentColor = FLinearColor(0.843f, 0.8f, 0.784f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Palette")
	FName PresetName = NAME_None;

	FPACivilianPalettePreset() = default;

	FPACivilianPalettePreset(FName InZoneId, const FLinearColor& InPrimary, const FLinearColor& InSecondary, const FLinearColor& InAccent, FName InPresetName)
		: ZoneId(InZoneId)
		, PrimaryColor(InPrimary)
		, SecondaryColor(InSecondary)
		, AccentColor(InAccent)
		, PresetName(InPresetName)
	{
	}
};

/**
 * FPACivilianConstants
 *
 * Hằng số định danh đạo cụ, Master Rig mapping và palette zones.
 */
struct PROJECTASCENDANT_API FPACivilianConstants
{
	// 10 Đạo cụ tĩnh 1-frame gắn tay (GDD §6.2)
	static const FName Prop_Blacksmith_Hammer; // Tay phải Thợ Rèn
	static const FName Prop_Tongs;             // Tay trái Thợ Rèn
	static const FName Prop_Merchant_GoldPouch;// Tay phải Thương Nhân
	static const FName Prop_Merchant_Scale;    // Tay trái Thương Nhân
	static const FName Prop_Villager_Basket;   // Tay phải Dân Làng
	static const FName Prop_Villager_Broom;    // Biến thể tay phải Dân Làng
	static const FName Prop_Guard_CitySpear;   // Tay phải Lính Gác
	static const FName Prop_Guard_CityShield;  // Tay trái Lính Gác
	static const FName Prop_Quest_Scroll;      // Tay phải Quest Giver
	static const FName Prop_Quest_Lantern;     // Tay trái Quest Giver

	// VFX Callout của Quest Giver
	static const FName VFX_Quest_Exclamation;  // Biểu tượng '!' vàng
	static const FName FB_QuestGiver_Wave;     // Frame hoạt ảnh vẫy tay

	// Master Material
	static const FName MasterMaterialName;     // M_PaperZD_Civilian_Base

	// Bán kính phát hiện Outlaw của Lính Gác (1000 cm)
	static constexpr float GuardOutlawDetectionRadius = 1000.0f;
	// Lực đẩy lùi Knockback khi Lính Gác trừng phạt Outlaw
	static constexpr float GuardLethalStrikeKnockback = 800.0f;

	/** Mapping 5 vai trò vào đúng 1 trong 4 Master Rigs */
	static EPAMasterRig GetMasterRigForCivilianRole(EPACivilianRole Role);

	/** Lấy đạo cụ mặc định cho từng vai trò */
	static FName GetDefaultRightPropForRole(EPACivilianRole Role);
	static FName GetDefaultLeftPropForRole(EPACivilianRole Role);

	/** Lấy preset bảng màu theo Zone */
	static FPACivilianPalettePreset GetPalettePresetForZone(FName ZoneId);
};

/**
 * FPACivilianNPCModel
 *
 * Pure data model quản lý trạng thái, role, palette, đạo cụ và AI lính gác của Civilian NPC.
 * 100% headless testable.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACivilianNPCModel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	EPACivilianRole Role = EPACivilianRole::Villager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	EPAMasterRig MasterRig = EPAMasterRig::Agility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	FName AttachedRightProp = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	FName AttachedLeftProp = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	FName CurrentZoneId = FName(TEXT("Citadel_VerdantBastion"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	FPACivilianPalettePreset CurrentPalette;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	EPAGuardAIState GuardState = EPAGuardAIState::Passive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	FName UpperBodyVisualAssetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	FName LowerBodyVisualAssetId = FName(TEXT("FB_Lower_Agility_Set"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	bool bCalloutActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	FName ActiveCalloutVisualId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian|Model")
	bool bLethalStrikeTriggered = false;

	FPACivilianNPCModel()
	{
		SetRole(EPACivilianRole::Villager);
		SetZone(FName(TEXT("Citadel_VerdantBastion")));
	}

	/** Thiết lập vai trò NPC, tự động map Master Rig và gán đạo cụ mặc định */
	void SetRole(EPACivilianRole InRole)
	{
		Role = InRole;
		MasterRig = FPACivilianConstants::GetMasterRigForCivilianRole(InRole);
		LowerBodyVisualAssetId = FPAPaperdollConstants::GetDefaultLowerBodyAssetForRig(MasterRig);
		AttachedRightProp = FPACivilianConstants::GetDefaultRightPropForRole(InRole);
		AttachedLeftProp = FPACivilianConstants::GetDefaultLeftPropForRole(InRole);
		GuardState = EPAGuardAIState::Passive;
		bLethalStrikeTriggered = false;

		if (InRole == EPACivilianRole::TownGuard)
		{
			// Mặc định lính gác bồng giáo
			UpperBodyVisualAssetId = FName(TEXT("FB_Upper_Guard_Spear_Passive"));
		}
		else
		{
			UpperBodyVisualAssetId = NAME_None;
		}
	}

	/** Nạp Palette màu theo ZoneId */
	void SetZone(FName InZoneId)
	{
		CurrentZoneId = InZoneId;
		CurrentPalette = FPACivilianConstants::GetPalettePresetForZone(InZoneId);
	}

	/** Gắn đạo cụ vào tay */
	void AttachProp(FName PropVisualId, bool bLeftHand)
	{
		if (bLeftHand)
		{
			AttachedLeftProp = PropVisualId;
		}
		else
		{
			AttachedRightProp = PropVisualId;
		}
	}

	FName GetAttachedProp(bool bLeftHand) const
	{
		return bLeftHand ? AttachedLeftProp : AttachedRightProp;
	}

	/** Kích hoạt Callout vẫy tay của Quest Giver */
	void PlayCalloutAnimation()
	{
		bCalloutActive = true;
		ActiveCalloutVisualId = FPACivilianConstants::VFX_Quest_Exclamation;
		UpperBodyVisualAssetId = FPACivilianConstants::FB_QuestGiver_Wave;
	}

	void StopCalloutAnimation()
	{
		bCalloutActive = false;
		ActiveCalloutVisualId = NAME_None;
		UpperBodyVisualAssetId = NAME_None;
	}

	/** Đánh giá mối đe dọa từ mục tiêu (Town Guard AI) */
	bool EvaluateTargetThreat(const FVector& SelfLocation, const FVector& TargetLocation, bool bTargetIsOutlaw, float DetectionRadius = FPACivilianConstants::GuardOutlawDetectionRadius)
	{
		if (Role != EPACivilianRole::TownGuard)
		{
			return false;
		}

		const float Dist2D = FVector::Dist2D(SelfLocation, TargetLocation);
		if (bTargetIsOutlaw && Dist2D <= DetectionRadius)
		{
			// Kích hoạt Lethal Gatekeeper Strike
			GuardState = EPAGuardAIState::CombatActive;
			// Tái sử dụng trực tiếp hoạt ảnh đòn đâm của Weapon Family 3 (FB_Upper_2H_Polearm_Combo)
			UpperBodyVisualAssetId = FPAPaperdollConstants::GetDefaultUpperBodyAssetForFamily(EPAWeaponFamily::Polearm_2H);
			AttachedRightProp = FPACivilianConstants::Prop_Guard_CitySpear;
			bLethalStrikeTriggered = true;
			return true;
		}

		// Không phát hiện outlaw trong bán kính an toàn -> Duy trì passive
		if (GuardState == EPAGuardAIState::CombatActive && Dist2D > DetectionRadius)
		{
			GuardState = EPAGuardAIState::Passive;
			UpperBodyVisualAssetId = FName(TEXT("FB_Upper_Guard_Spear_Passive"));
			bLethalStrikeTriggered = false;
		}

		return false;
	}
};
