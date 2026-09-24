// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/PAInventoryTypes.h"
#include "PABlacksmithTypes.generated.h"

/**
 * EPABlacksmithTier
 *
 * 3 cấp độ lò rèn phân vùng dã ngoại theo GDD (blacksmithing-system.md):
 * - Tier 1 (Outpost Forge): Khu an toàn / Tiền trạm, sửa chữa, phân rã, cường hóa an toàn tối đa +3 (100%).
 * - Tier 2 (Wilderness Forge): Rừng sâu hoang dã, cường hóa tối đa +6, đục 2 lỗ khảm ngọc.
 * - Tier 3 (Ancient Sanctuary Forge): Hang Lãnh Chúa, cường hóa đỉnh phong +10, đục lỗ Prismatic, đúc Thần Binh từ Boss Soul.
 */
UENUM(BlueprintType)
enum class EPABlacksmithTier : uint8
{
	None              = 0 UMETA(DisplayName = "None / Unassigned"),
	Tier1_Outpost     = 1 UMETA(DisplayName = "Tier 1: Outpost Forge"),
	Tier2_Wilderness  = 2 UMETA(DisplayName = "Tier 2: Wilderness Forge"),
	Tier3_Sanctuary   = 3 UMETA(DisplayName = "Tier 3: Ancient Sanctuary Forge")
};

/**
 * EPACraftingError
 *
 * Mã lỗi phản hồi khi thực hiện các nghiệp vụ thợ rèn (Sửa chữa, Phân rã, Cường hóa).
 */
UENUM(BlueprintType)
enum class EPACraftingError : uint8
{
	None                  = 0 UMETA(DisplayName = "None"),
	InsufficientGold      = 1 UMETA(DisplayName = "Insufficient Gold"),
	InsufficientMaterials = 2 UMETA(DisplayName = "Insufficient Materials"),
	ItemNotFound          = 3 UMETA(DisplayName = "Item Not Found"),
	ItemLocked            = 4 UMETA(DisplayName = "Item Is Locked"),
	MaxDurabilityAlready  = 5 UMETA(DisplayName = "Item Already At Max Durability"),
	MaxTierLevelReached   = 6 UMETA(DisplayName = "Forge Tier Enhancement Limit Reached"),
	DistanceExceeded      = 7 UMETA(DisplayName = "Interaction Distance Exceeded (>300cm)"),
	InCombat              = 8 UMETA(DisplayName = "Cannot Use Forge In Combat"),
	InvalidItemType       = 9 UMETA(DisplayName = "Invalid Item Type For Operation"),
	ServerRejected        = 10 UMETA(DisplayName = "Server Rejected Request"),
	ItemCannotBeSocketed  = 11 UMETA(DisplayName = "Item Cannot Be Socketed (Must Be Rare+ Equipment)"),
	MaxSocketsReached     = 12 UMETA(DisplayName = "Maximum Gem Sockets Reached"),
	InvalidSocketIndex    = 13 UMETA(DisplayName = "Invalid Socket Index"),
	SocketNotEmpty        = 14 UMETA(DisplayName = "Socket Already Contains A Gem"),
	SocketEmpty           = 15 UMETA(DisplayName = "Socket Is Empty"),
	InvalidGemItem        = 16 UMETA(DisplayName = "Invalid Gem Item"),
	InventoryFull         = 17 UMETA(DisplayName = "Inventory Full"),
	NoWardItem            = 18 UMETA(DisplayName = "No Blacksmith Ward Item In Inventory"),
	MaxBackpackCapacity   = 19 UMETA(DisplayName = "Maximum Backpack Capacity Reached (60 Slots)"),
	BackpackSequenceMismatch = 20 UMETA(DisplayName = "Backpack Must Be Upgraded Sequentially (40 -> 50 -> 60)"),
	InsufficientSkillShards  = 21 UMETA(DisplayName = "Insufficient Skill Shards")
};

/**
 * FPABackpackExpansionRequirements
 *
 * Yêu cầu tài nguyên và Bậc Lò Rèn để nâng cấp sức chứa kho đồ (GDD blacksmithing-system.md §D, Story crft-003).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPABackpackExpansionRequirements
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blacksmith|Backpack")
	int32 TargetCapacity = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blacksmith|Backpack")
	int32 GoldCost = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blacksmith|Backpack")
	EPABlacksmithTier RequiredForgeTier = EPABlacksmithTier::Tier1_Outpost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blacksmith|Backpack")
	FName MaterialItemA = FName("leather");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blacksmith|Backpack")
	int32 MaterialCountA = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blacksmith|Backpack")
	FName MaterialItemB = FName("copper_ore");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blacksmith|Backpack")
	int32 MaterialCountB = 5;
};

/**
 * FPABlacksmithFormulas
 *
 * Bộ công thức toán học và bảng tỷ lệ rèn đúc chuẩn xác theo GDD blacksmithing-system.md.
 */
struct PROJECTASCENDANT_API FPABlacksmithFormulas
{
	/**
	 * AC-1: Tính chi phí sửa chữa độ bền bằng Vàng:
	 * repair_cost = ceil(base_price * 0.25 * (1.0 - (current_durability / max_durability)))
	 */
	static int32 CalculateRepairCost(int32 BasePrice, float CurrentDurability, float MaxDurability = 100.0f)
	{
		if (BasePrice <= 0 || CurrentDurability >= MaxDurability)
		{
			return 0;
		}

		const double SafeMax = FMath::Max(1.0, static_cast<double>(MaxDurability));
		const double MissingRatio = FMath::Clamp((SafeMax - static_cast<double>(CurrentDurability)) / SafeMax, 0.0, 1.0);

		const double RawCost = static_cast<double>(BasePrice) * 0.25 * MissingRatio;
		return FMath::Max(1, FMath::CeilToInt(static_cast<float>(RawCost - 1e-4)));
	}

	/**
	 * AC-2: Sản lượng Tàn Trang (Ash Shards) khi phân rã trang bị hoặc Sách Kỹ Năng:
	 * - Skill Book: Cố định 5 Ash Shards
	 * - Common (Tier 1): 1 Ash Shards
	 * - Uncommon (Tier 2): 3 Ash Shards
	 * - Rare (Tier 3): 10 Ash Shards
	 * - Epic (Tier 4): 25 Ash Shards
	 * - Legendary (Tier 5): 75 Ash Shards
	 */
	static int32 GetSalvageAshShards(EPAItemRarity Rarity, EPAItemCategory Category)
	{
		if (Category == EPAItemCategory::SkillBook)
		{
			return 5;
		}

		switch (Rarity)
		{
		case EPAItemRarity::Common:
			return 1;
		case EPAItemRarity::Uncommon:
			return 3;
		case EPAItemRarity::Rare:
			return 10;
		case EPAItemRarity::Epic:
			return 25;
		case EPAItemRarity::Legendary:
			return 75;
		default:
			return 1;
		}
	}

	/**
	 * AC-3: Yêu cầu Vàng và Quặng Sắt khi cường hóa an toàn (+0 -> +3):
	 * - +0 -> +1: 100 Gold + 2 Iron Ore
	 * - +1 -> +2: 250 Gold + 4 Iron Ore
	 * - +2 -> +3: 500 Gold + 8 Iron Ore
	 */
	static bool GetEnhancementCost(int32 CurrentLevel, int32& OutGoldCost, int32& OutIronOreCost)
	{
		switch (CurrentLevel)
		{
		case 0:
			OutGoldCost = 100;
			OutIronOreCost = 2;
			return true;
		case 1:
			OutGoldCost = 250;
			OutIronOreCost = 4;
			return true;
		case 2:
			OutGoldCost = 500;
			OutIronOreCost = 8;
			return true;
		case 3:
			OutGoldCost = 800;
			OutIronOreCost = 12;
			return true;
		case 4:
			OutGoldCost = 1200;
			OutIronOreCost = 16;
			return true;
		case 5:
			OutGoldCost = 1800;
			OutIronOreCost = 20;
			return true;
		case 6:
			OutGoldCost = 2500;
			OutIronOreCost = 25;
			return true;
		case 7:
			OutGoldCost = 3500;
			OutIronOreCost = 30;
			return true;
		case 8:
			OutGoldCost = 5000;
			OutIronOreCost = 40;
			return true;
		case 9:
			OutGoldCost = 7500;
			OutIronOreCost = 50;
			return true;
		default:
			OutGoldCost = 0;
			OutIronOreCost = 0;
			return false;
		}
	}

	/**
	 * Giới hạn cấp độ cường hóa theo Bậc Lò Rèn:
	 * - Tier 1: Max +3
	 * - Tier 2: Max +6
	 * - Tier 3: Max +10
	 */
	static int32 GetMaxEnhancementForTier(EPABlacksmithTier Tier)
	{
		switch (Tier)
		{
		case EPABlacksmithTier::Tier1_Outpost:
			return 3;
		case EPABlacksmithTier::Tier2_Wilderness:
			return 6;
		case EPABlacksmithTier::Tier3_Sanctuary:
			return 10;
		default:
			return 3;
		}
	}

	/**
	 * Tăng trưởng sát thương vũ khí khi cường hóa:
	 * WeaponAttack(Level) = round(BaseAttack * (1.0 + 0.05 * Level)) + (Level == 10 ? round(BaseAttack * 0.10) : 0)
	 */
	static float CalculateWeaponAttack(float BaseAttack, int32 Level)
	{
		if (Level <= 0)
		{
			return BaseAttack;
		}

		float Multiplier = 1.0f + 0.05f * Level;
		float Scaled = BaseAttack * Multiplier;
		if (Level >= 10)
		{
			Scaled += BaseAttack * 0.10f;
		}
		return FMath::RoundToFloat(Scaled);
	}

	/**
	 * Tăng trưởng giáp phòng ngự khi cường hóa:
	 * ArmorDefense(Level) = round(BaseDefense * (1.0 + 0.06 * Level))
	 */
	static float CalculateArmorDefense(float BaseDefense, int32 Level)
	{
		if (Level <= 0)
		{
			return BaseDefense;
		}

		return FMath::RoundToFloat(BaseDefense * (1.0f + 0.06f * Level));
	}

	/**
	 * AC-1 & AC-2: Tỷ lệ thành công khi cường hóa (+1 -> +10):
	 * - +1 -> +3: 100% (1.00)
	 * - +4: 70% (0.70)
	 * - +5: 60% (0.60)
	 * - +6: 50% (0.50)
	 * - +7: 40% (0.40)
	 * - +8: 30% (0.30)
	 * - +9: 25% (0.25)
	 * - +10: 15% (0.15)
	 */
	static float GetEnhancementSuccessRate(int32 TargetLevel)
	{
		if (TargetLevel <= 3)
		{
			return 1.0f;
		}

		switch (TargetLevel)
		{
		case 4: return 0.70f;
		case 5: return 0.60f;
		case 6: return 0.50f;
		case 7: return 0.40f;
		case 8: return 0.30f;
		case 9: return 0.25f;
		case 10: return 0.15f;
		default: return 0.0f;
		}
	}

	/**
	 * AC-3: Giới hạn số lỗ ngọc có thể đục trên trang bị:
	 * - Chỉ trang bị Bậc Rare trở lên mới có thể đục lỗ.
	 * - Tier 1: 0 lỗ
	 * - Tier 2: Tối đa 2 lỗ
	 * - Tier 3: Tối đa 3 lỗ
	 */
	static int32 GetMaxGemSockets(EPABlacksmithTier ForgeTier, EPAItemRarity Rarity)
	{
		if (Rarity < EPAItemRarity::Rare)
		{
			return 0;
		}

		switch (ForgeTier)
		{
		case EPABlacksmithTier::Tier2_Wilderness:
			return 2;
		case EPABlacksmithTier::Tier3_Sanctuary:
			return 3;
		default:
			return 0;
		}
	}

	/**
	 * AC-3: Phí tháo ngọc cố định: 100 Vàng.
	 */
	static int32 GetUnsocketGoldFee()
	{
		return 100;
	}

	/**
	 * AC-2 (crft-003): Kiểm tra có phải Ngọc Đa Sắc (Prismatic Gem) hay không.
	 */
	static bool IsPrismaticGem(FName ItemId)
	{
		return ItemId == FName("prismatic_gem") || ItemId == FName("gem_prismatic");
	}

	/**
	 * AC-2 (crft-003): Kiểm tra SocketIndex có phải là Lỗ Khảm Đa Sắc (Prismatic Socket) thứ 3 hay không.
	 */
	static bool IsPrismaticSocket(int32 SocketIndex)
	{
		return SocketIndex == 2;
	}

	/**
	 * AC-3 (crft-002) / AC-2 (crft-003): Kiểm tra ItemId có phải là Ngọc Khảm hợp lệ không.
	 */
	static bool IsGemItem(FName ItemId)
	{
		return ItemId == FName("ruby") || ItemId == FName("gem_ruby") ||
		       ItemId == FName("sapphire") || ItemId == FName("gem_sapphire") ||
		       ItemId == FName("topaz") || ItemId == FName("gem_topaz") ||
		       IsPrismaticGem(ItemId);
	}

	/**
	 * AC-1 (crft-003): Định mức chi phí và số lượng nguyên liệu đúc Thần Binh từ Linh Hồn Boss:
	 * - 1x Linh Hồn Lãnh Chúa (Boss Soul)
	 * - 4x Mảnh vỡ bộ phận Boss (Boss Parts)
	 * - 5x Quặng Hư Không Cổ Đại (Void Ore)
	 * - 5,000 Vàng
	 */
	static void GetBossSoulForgingCost(int32& OutGoldCost, int32& OutVoidOreCount, int32& OutBossPartCount, int32& OutBossSoulCount)
	{
		OutGoldCost = 5000;
		OutVoidOreCount = 5;
		OutBossPartCount = 4;
		OutBossSoulCount = 1;
	}

	static bool IsBossSoulItem(FName ItemId)
	{
		const FString Str = ItemId.ToString();
		return Str.Contains(TEXT("boss_soul")) || ItemId == FName("boss_soul") || ItemId == FName("item_boss_soul");
	}

	static bool IsBossPartItem(FName ItemId)
	{
		const FString Str = ItemId.ToString();
		return Str.Contains(TEXT("boss_part")) || Str.Contains(TEXT("boss_horn")) || Str.Contains(TEXT("boss_scale")) || Str.Contains(TEXT("boss_wing")) || ItemId == FName("boss_part") || ItemId == FName("item_boss_part");
	}

	static bool IsVoidOreItem(FName ItemId)
	{
		const FString Str = ItemId.ToString();
		return Str.Contains(TEXT("void_ore")) || ItemId == FName("void_ore") || ItemId == FName("item_ore_void") || ItemId == FName("item_void_ore");
	}

	/**
	 * AC-3 (crft-003): Lấy yêu cầu nguyên liệu và Bậc Lò Rèn cho từng mốc nâng cấp túi đồ:
	 * - 30 -> 40: Tier 1 Forge, 500 Gold, 10 Da Thú, 5 Quặng Đồng
	 * - 40 -> 50: Tier 2 Forge, 2000 Gold, 15 Da Cường Lực, 5 Quặng Sắt Đen
	 * - 50 -> 60: Tier 3 Forge, 8000 Gold, 5 Vảy Đuôi Boss, 2 Quặng Hư Không
	 */
	static bool GetBackpackExpansionRequirement(int32 CurrentSlots, FPABackpackExpansionRequirements& OutReq)
	{
		if (CurrentSlots < 40)
		{
			OutReq.TargetCapacity = 40;
			OutReq.GoldCost = 500;
			OutReq.RequiredForgeTier = EPABlacksmithTier::Tier1_Outpost;
			OutReq.MaterialItemA = FName("leather");
			OutReq.MaterialCountA = 10;
			OutReq.MaterialItemB = FName("copper_ore");
			OutReq.MaterialCountB = 5;
			return true;
		}
		else if (CurrentSlots < 50)
		{
			OutReq.TargetCapacity = 50;
			OutReq.GoldCost = 2000;
			OutReq.RequiredForgeTier = EPABlacksmithTier::Tier2_Wilderness;
			OutReq.MaterialItemA = FName("hardened_leather");
			OutReq.MaterialCountA = 15;
			OutReq.MaterialItemB = FName("black_iron_ore");
			OutReq.MaterialCountB = 5;
			return true;
		}
		else if (CurrentSlots < 60)
		{
			OutReq.TargetCapacity = 60;
			OutReq.GoldCost = 8000;
			OutReq.RequiredForgeTier = EPABlacksmithTier::Tier3_Sanctuary;
			OutReq.MaterialItemA = FName("boss_scale");
			OutReq.MaterialCountA = 5;
			OutReq.MaterialItemB = FName("void_ore");
			OutReq.MaterialCountB = 2;
			return true;
		}

		return false; // Đã đạt trần tối đa 60 ô
	}
};
