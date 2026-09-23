// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PAZoneTypes.generated.h"

/**
 * EPAZoneTier
 *
 * 3 phân vùng thế giới mở nối liền (World Partition) theo GDD zone-system.md:
 * - Tier1_VerdantFrontier: Lv 1-15, Outpost an toàn, guards Lv 50, PvP bị cấm trong trấn.
 * - Tier2_AshenWilderness: Lv 16-30, Rừng tro tàn, đầm lầy độc, quái tinh anh tuần tra 4-6 con, open PvP.
 * - Tier3_ForbiddenSanctum: Lv 31-50, Vùng cấm sương mù dày đặc, khe nứt không gian, Lò rèn Boss Soul.
 */
UENUM(BlueprintType)
enum class EPAZoneTier : uint8
{
	Tier1_VerdantFrontier   = 1 UMETA(DisplayName = "Tier 1: Verdant Frontier (Lv 1-15)"),
	Tier2_AshenWilderness   = 2 UMETA(DisplayName = "Tier 2: Ashen Wilderness (Lv 16-30)"),
	Tier3_ForbiddenSanctum  = 3 UMETA(DisplayName = "Tier 3: Forbidden Sanctum (Lv 31-50)")
};

/**
 * EPACampfireState
 *
 * Trạng thái của Đống Lửa (Campfire):
 * - Dormant: Chưa được thắp sáng.
 * - Active: Đã thắp sáng, cung cấp Sanctuary 1000cm và điểm lưu.
 * - Resting: Người chơi đang ngồi nghỉ (hồi phục 100% tài nguyên).
 * - FastTraveling: Đang niệm chú dịch chuyển 2.0s.
 */
UENUM(BlueprintType)
enum class EPACampfireState : uint8
{
	Dormant        = 0 UMETA(DisplayName = "Dormant (Unlit)"),
	Active         = 1 UMETA(DisplayName = "Active (Lit Sanctuary)"),
	Resting        = 2 UMETA(DisplayName = "Resting"),
	FastTraveling  = 3 UMETA(DisplayName = "Fast Traveling (2.0s Cast)")
};

/**
 * FPACampfireNode
 *
 * Dữ liệu của một điểm Đống Lửa trong thế giới.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACampfireNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Campfire")
	FName CampfireId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Campfire")
	EPAZoneTier ZoneTier = EPAZoneTier::Tier1_VerdantFrontier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Campfire")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Campfire")
	bool bIsLit = false;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Campfire")
	bool bIsCurrentSpawnAnchor = false;

	FPACampfireNode() = default;
};

/**
 * FPAZoneConfig
 *
 * Tham số cân chỉnh vùng và thánh địa theo GDD zone-system.md §Tuning Knobs.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAZoneConfig
{
	GENERATED_BODY()

	/** Bán kính vùng an toàn Thánh địa (1000cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Config")
	float SanctuaryRadius = 1000.0f;

	/** Cự ly tối đa quái vật được phép di chuyển khỏi vị trí sinh (2500cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Config")
	float MaxLeashDistance = 2500.0f;

	/** Hệ số tăng tốc độ khi quái vật quay về tổ (+50% = 1.5x) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Config")
	float LeashSpeedMultiplier = 1.50f;

	/** Thời gian niệm chú dịch chuyển nhanh (2.0s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Config")
	float FastTravelCastDuration = 2.0f;

	/** Số lượt dùng bình dược phẩm tối đa */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Config")
	int32 MaxFlaskCharges = 5;
};

/**
 * FPAZoneModel
 *
 * Pure data model quản lý danh sách Đống Lửa, kiểm tra ranh giới Sanctuary 1000cm,
 * tiến trình Fast Travel 2.0s (hủy nếu di chuyển/nhận sát thương), và AI Leash 2500cm.
 * 100% headless testable.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAZoneModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Model")
	TArray<FPACampfireNode> Campfires;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Model")
	FName CurrentSpawnAnchorId;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Model")
	EPACampfireState CurrentState = EPACampfireState::Dormant;

	/** Điểm đến Fast Travel đang niệm chú */
	UPROPERTY(BlueprintReadOnly, Category = "Zone|Model")
	FName FastTravelDestinationId;

	/** Tiến trình niệm chú dịch chuyển (0.0s -> 2.0s) */
	UPROPERTY(BlueprintReadOnly, Category = "Zone|Model")
	float FastTravelElapsed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Model")
	bool bIsChannelingFastTravel = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Model")
	FPAZoneConfig Config;

	// ===========================================================
	// Init & Registration
	// ===========================================================

	FPAZoneModel() = default;

	void RegisterCampfire(FName Id, EPAZoneTier Tier, const FVector& Location, bool bInitiallyLit = false)
	{
		FPACampfireNode Node;
		Node.CampfireId = Id;
		Node.ZoneTier = Tier;
		Node.WorldLocation = Location;
		Node.bIsLit = bInitiallyLit;
		Node.bIsCurrentSpawnAnchor = false;
		Campfires.Add(Node);
	}

	// ===========================================================
	// Campfire Actions
	// ===========================================================

	bool LightCampfire(FName Id)
	{
		FPACampfireNode* Node = FindCampfire(Id);
		if (!Node)
		{
			return false;
		}

		Node->bIsLit = true;
		SetSpawnAnchor(Id);
		return true;
	}

	void SetSpawnAnchor(FName Id)
	{
		CurrentSpawnAnchorId = Id;
		for (FPACampfireNode& Node : Campfires)
		{
			Node.bIsCurrentSpawnAnchor = (Node.CampfireId == Id);
		}
	}

	/**
	 * Nghỉ ngơi tại đống lửa: Hồi 100% tài nguyên và đầy bình dược phẩm.
	 */
	bool RestAtCampfire(FName Id, float& OutHP, float& OutMP, float& OutStamina, int32& OutFlasks)
	{
		FPACampfireNode* Node = FindCampfire(Id);
		if (!Node || !Node->bIsLit)
		{
			return false;
		}

		SetSpawnAnchor(Id);
		OutHP = 1.0f; // 100%
		OutMP = 1.0f; // 100%
		OutStamina = 1.0f; // 100%
		OutFlasks = Config.MaxFlaskCharges; // 5/5
		return true;
	}

	// ===========================================================
	// Fast Travel (2.0s Cast)
	// ===========================================================

	bool StartFastTravel(FName FromId, FName ToId, bool bInCombat)
	{
		if (bInCombat || bIsChannelingFastTravel)
		{
			return false;
		}

		const FPACampfireNode* From = FindCampfire(FromId);
		const FPACampfireNode* To = FindCampfire(ToId);

		// Cả 2 điểm phải tồn tại và đều đã được thắp sáng
		if (!From || !To || !From->bIsLit || !To->bIsLit)
		{
			return false;
		}

		bIsChannelingFastTravel = true;
		FastTravelDestinationId = ToId;
		FastTravelElapsed = 0.0f;
		CurrentState = EPACampfireState::FastTraveling;
		return true;
	}

	/**
	 * Cập nhật tiến trình niệm chú dịch chuyển.
	 * @return true nếu niệm chú hoàn tất (dịch chuyển thành công).
	 */
	bool UpdateFastTravel(float DeltaTime, bool bPlayerMoved, bool bPlayerDamaged)
	{
		if (!bIsChannelingFastTravel)
		{
			return false;
		}

		// Bị ngắt quãng bởi di chuyển hoặc nhận sát thương
		if (bPlayerMoved || bPlayerDamaged)
		{
			CancelFastTravel();
			return false;
		}

		FastTravelElapsed += DeltaTime;

		if (FastTravelElapsed >= Config.FastTravelCastDuration)
		{
			CompleteFastTravel();
			return true; // Dịch chuyển thành công!
		}

		return false;
	}

	void CancelFastTravel()
	{
		bIsChannelingFastTravel = false;
		FastTravelDestinationId = NAME_None;
		FastTravelElapsed = 0.0f;
		CurrentState = EPACampfireState::Active;
	}

	void CompleteFastTravel()
	{
		bIsChannelingFastTravel = false;
		FastTravelElapsed = 0.0f;
		CurrentState = EPACampfireState::Active;
		SetSpawnAnchor(FastTravelDestinationId);
	}

	// ===========================================================
	// Sanctuary & AI Leash Boundary Math
	// ===========================================================

	/**
	 * Kiểm tra xem vị trí có nằm trong Thánh địa 1000cm của bất kỳ Campfire nào đã thắp sáng không.
	 */
	bool IsInsideSanctuary(const FVector& Location, FName& OutCampfireId) const
	{
		const float RadiusSq = FMath::Square(Config.SanctuaryRadius);

		for (const FPACampfireNode& Node : Campfires)
		{
			if (Node.bIsLit)
			{
				const float DistSq = FVector::DistSquared2D(Location, Node.WorldLocation);
				if (DistSq <= RadiusSq)
				{
					OutCampfireId = Node.CampfireId;
					return true;
				}
			}
		}

		OutCampfireId = NAME_None;
		return false;
	}

	/**
	 * Kiểm tra xem quái vật có vượt quá ranh giới Leash 2500cm so với vị trí sinh không.
	 */
	bool IsBeyondLeash(const FVector& CurrentLocation, const FVector& SpawnOrigin) const
	{
		const float DistSq = FVector::DistSquared2D(CurrentLocation, SpawnOrigin);
		return DistSq > FMath::Square(Config.MaxLeashDistance);
	}

	// ===========================================================
	// Helpers
	// ===========================================================

	FPACampfireNode* FindCampfire(FName Id)
	{
		for (FPACampfireNode& Node : Campfires)
		{
			if (Node.CampfireId == Id)
			{
				return &Node;
			}
		}
		return nullptr;
	}

	const FPACampfireNode* FindCampfire(FName Id) const
	{
		for (const FPACampfireNode& Node : Campfires)
		{
			if (Node.CampfireId == Id)
			{
				return &Node;
			}
		}
		return nullptr;
	}
};
