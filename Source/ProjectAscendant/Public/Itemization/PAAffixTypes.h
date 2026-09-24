// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PAAffixTypes.generated.h"

/**
 * EPAAffixType
 *
 * Phân loại thuộc tính trang bị theo GDD itemization.md:
 * - Prefix: Tiền tố (thiên về công / sát thương / bùa hại)
 * - Suffix: Hậu tố (thiên về thủ / hồi phục / cơ động)
 */
UENUM(BlueprintType)
enum class EPAAffixType : uint8
{
	Prefix = 0 UMETA(DisplayName = "Prefix (Tiền Tố)"),
	Suffix = 1 UMETA(DisplayName = "Suffix (Hậu Tố)")
};

/**
 * EPAForgeTier
 *
 * 3 Bậc Lò Rèn chuẩn hóa toàn dự án theo itemization.md:
 * - Tier 1: Outpost Forge (Lò Rèn Tiền Trạm, iLvl 1-20, 8P/8S)
 * - Tier 2: Field Forge (Lò Rèn Dã Ngoại, iLvl 21-35, 11P/11S lũy tiến)
 * - Tier 3: Forbidden Forge (Lò Rèn Cấm Địa, iLvl 36-50, 14P/14S lũy tiến)
 */
UENUM(BlueprintType)
enum class EPAForgeTier : uint8
{
	None              = 0 UMETA(DisplayName = "None / Unassigned"),
	Tier1_Outpost     = 1 UMETA(DisplayName = "Tier 1: Outpost Forge"),
	Tier2_Field       = 2 UMETA(DisplayName = "Tier 2: Field Forge"),
	Tier3_Forbidden   = 3 UMETA(DisplayName = "Tier 3: Forbidden Forge")
};

/**
 * EPASocketType
 *
 * Loại lỗ khảm ngọc trên trang bị (Story item-003):
 * - Regular: Lỗ thường (mở tại Field Forge cho đồ Rare/Epic)
 * - Prismatic: Lỗ Đa Sắc (mở tại Forbidden Forge độc quyền cho Legendary)
 */
UENUM(BlueprintType)
enum class EPASocketType : uint8
{
	None        = 0 UMETA(DisplayName = "None"),
	Regular     = 1 UMETA(DisplayName = "Regular Socket"),
	Prismatic   = 2 UMETA(DisplayName = "Prismatic Socket")
};

/**
 * FPASocketSlot
 *
 * Dữ liệu cho 1 ô lỗ khảm ngọc trên trang bị biến đổi.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPASocketSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Socket")
	EPASocketType SocketType = EPASocketType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Socket")
	FName SocketedGemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Socket")
	bool bIsUnlocked = false;

	FPASocketSlot()
		: SocketType(EPASocketType::None)
		, SocketedGemId(NAME_None)
		, bIsUnlocked(false)
	{
	}

	FPASocketSlot(EPASocketType InType, bool bInUnlocked = false)
		: SocketType(InType)
		, SocketedGemId(NAME_None)
		, bIsUnlocked(bInUnlocked)
	{
	}

	bool HasGem() const { return !SocketedGemId.IsNone(); }

	friend FArchive& operator<<(FArchive& Ar, FPASocketSlot& Slot)
	{
		uint8 TypeByte = static_cast<uint8>(Slot.SocketType);
		Ar << TypeByte;
		if (Ar.IsLoading())
		{
			Slot.SocketType = static_cast<EPASocketType>(TypeByte);
		}
		Ar << Slot.SocketedGemId;
		Ar << Slot.bIsUnlocked;
		return Ar;
	}
};

/**
 * FPAAffixInstance
 *
 * Dòng Affix cụ thể đã được xúc xắc và gán vào một món trang bị.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAAffixInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	FName AffixId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	EPAAffixType AffixType = EPAAffixType::Prefix;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	float RolledValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	int32 AffixTier = 1; // Bậc xúc xắc 1..4 tương ứng dải iLvl

	FPAAffixInstance()
		: AffixId(NAME_None)
		, AffixType(EPAAffixType::Prefix)
		, RolledValue(0.0f)
		, AffixTier(1)
	{
	}

	FPAAffixInstance(FName InId, EPAAffixType InType, float InValue, int32 InTier)
		: AffixId(InId)
		, AffixType(InType)
		, RolledValue(InValue)
		, AffixTier(InTier)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FPAAffixInstance& Instance)
	{
		Ar << Instance.AffixId;
		uint8 TypeByte = static_cast<uint8>(Instance.AffixType);
		Ar << TypeByte;
		if (Ar.IsLoading())
		{
			Instance.AffixType = static_cast<EPAAffixType>(TypeByte);
		}
		Ar << Instance.RolledValue;
		Ar << Instance.AffixTier;
		return Ar;
	}
};

/**
 * FPAAffixDefinitionRow
 *
 * Cấu trúc dòng DataTable DT_AffixDefinitions nạp 28 Affix từ entities.yaml:475.
 * MinForgeTier được định nghĩa chuẩn là bậc lò rèn TỐI THIỂU để mở khóa:
 * - Khi CurrentForgeTier >= MinForgeTier: Affix này hợp lệ trong pool roll.
 * - Outpost: 8P + 8S (MinForgeTier <= Tier1_Outpost)
 * - Field: 11P + 11S (MinForgeTier <= Tier2_Field, lũy tiến 8 gốc + 3 mới)
 * - Forbidden: 14P + 14S (MinForgeTier <= Tier3_Forbidden, lũy tiến 11 trước + 3 mới)
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAAffixDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	FName AffixName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	EPAAffixType AffixType = EPAAffixType::Prefix;

	/** Bậc Lò Rèn tối thiểu mở khóa dòng này vào pool roll (Cấu trúc lũy tiến cộng dồn) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	EPAForgeTier MinForgeTier = EPAForgeTier::Tier1_Outpost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	FName TargetStat = NAME_None;

	/** Trần cứng tuyệt đối nếu có (ví dụ: prefix_posture_dmg trần 35.0%, prefix_stagger_duration trần 0.5s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	float HardCapPct = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix")
	FString StackingRule;

	// Dải giá trị theo 4 Bậc iLvl: T1 (1-15), T2 (16-30), T3 (31-45), T4 (46-50). X = Min, Y = Max.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix|Tiers")
	FVector2D Tier1Range = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix|Tiers")
	FVector2D Tier2Range = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix|Tiers")
	FVector2D Tier3Range = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Itemization|Affix|Tiers")
	FVector2D Tier4Range = FVector2D::ZeroVector;

	/** Kiểm tra tính khả dụng lũy tiến: CurrentForgeTier >= MinForgeTier */
	bool IsUnlockedAtForgeTier(EPAForgeTier CurrentForgeTier) const
	{
		return static_cast<uint8>(CurrentForgeTier) >= static_cast<uint8>(MinForgeTier);
	}

	/** Trích xuất dải giá trị min/max theo tier. Trả về false nếu tier này không có sẵn */
	bool GetTierRange(int32 InTier, float& OutMin, float& OutMax) const
	{
		FVector2D TargetRange = FVector2D::ZeroVector;
		switch (InTier)
		{
		case 1: TargetRange = Tier1Range; break;
		case 2: TargetRange = Tier2Range; break;
		case 3: TargetRange = Tier3Range; break;
		case 4: TargetRange = Tier4Range; break;
		default: return false;
		}

		if (TargetRange.IsZero())
		{
			return false;
		}

		OutMin = TargetRange.X;
		OutMax = TargetRange.Y;
		return true;
	}
};
