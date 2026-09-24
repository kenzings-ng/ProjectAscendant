// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PAZoneTypes.generated.h"

/**
 * EPAZoneTier
 *
 * 3 phân vùng thế giới mở nối liền (World Partition) theo GDD zone-system.md:
 * - Tier1_VerdantFrontier: Lv 1-15, Tòa thành Verdant Bastion, guards Lv 50, Safe Zone tuyệt đối.
 * - Tier2_AshenWilderness: Lv 16-30, Rừng tro tàn, đầm lầy độc, quái tinh anh tuần tra 4-6 con, Pháo đài Ashen Keep.
 * - Tier3_ForbiddenSanctum: Lv 31-50, Vùng cấm sương mù dày đặc, khe nứt không gian, Thánh điện Sanctum Fortress.
 */
UENUM(BlueprintType)
enum class EPAZoneTier : uint8
{
	None                    = 0 UMETA(DisplayName = "None / Unassigned"),
	Tier1_VerdantFrontier   = 1 UMETA(DisplayName = "Tier 1: Verdant Frontier (Lv 1-15)"),
	Tier2_AshenWilderness   = 2 UMETA(DisplayName = "Tier 2: Ashen Wilderness (Lv 16-30)"),
	Tier3_ForbiddenSanctum  = 3 UMETA(DisplayName = "Tier 3: Forbidden Sanctum (Lv 31-50)")
};

/**
 * EPACitadelState
 *
 * Trạng thái của Tòa Thành (Citadel / Safe Zone):
 * - Undiscovered: Chưa từng bước chân vào.
 * - Discovered: Đã mở khóa vào danh sách thành trì đã ghé thăm.
 * - FastTraveling: Đang niệm chú dịch chuyển giữa các thành trì (2.0s).
 */
UENUM(BlueprintType)
enum class EPACitadelState : uint8
{
	Undiscovered   = 0 UMETA(DisplayName = "Undiscovered"),
	Discovered     = 1 UMETA(DisplayName = "Discovered & Active"),
	FastTraveling  = 2 UMETA(DisplayName = "Fast Traveling (2.0s Cast)")
};

/**
 * FPACitadelNode
 *
 * Dữ liệu của một Tòa Thành (Citadel / Pháo Đài An Toàn) trong thế giới.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACitadelNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Citadel")
	FName CitadelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Citadel")
	FString CitadelDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Citadel")
	EPAZoneTier ZoneTier = EPAZoneTier::Tier1_VerdantFrontier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Citadel")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Citadel")
	float SafeZoneRadius = 5000.0f; // Bán kính vùng an toàn của tòa thành

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Citadel")
	bool bIsDiscovered = false;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Citadel")
	bool bIsLastVisited = false;

	FPACitadelNode() = default;
};

/**
 * FPAAutoSaveRecord
 *
 * Bản ghi tự động lưu (Auto-Save) khi người chơi bước vào Tòa Thành.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAAutoSaveRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Save")
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Save")
	FName LastVisitedCitadelId;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Save")
	FVector SavedSpawnLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Save")
	float AutoSaveTimestamp = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Save")
	bool bSavedSuccessfully = false;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Save")
	float SavedHPPercent = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Save")
	float SavedMPPercent = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Save")
	float SavedStaminaPercent = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Save")
	int32 SavedFlaskCharges = 5;

	FPAAutoSaveRecord() = default;
};

/**
 * FPAZoneConfig
 *
 * Tham số cân chỉnh vùng và Tòa thành an toàn theo GDD zone-system.md.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAZoneConfig
{
	GENERATED_BODY()

	/** Bán kính vùng an toàn mặc định của Tòa thành (5000cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Config")
	float DefaultCitadelRadius = 5000.0f;

	/** Cự ly tối đa quái vật được phép di chuyển khỏi vị trí sinh (2500cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Config")
	float MaxLeashDistance = 2500.0f;

	/** Hệ số tăng tốc độ khi quái vật quay về tổ (+50% = 1.5x) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Config")
	float LeashSpeedMultiplier = 1.50f;

	/** Thời gian niệm chú dịch chuyển nhanh giữa các thành trì (2.0s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Config")
	float FastTravelCastDuration = 2.0f;

	/** Số lượt dùng bình dược phẩm tối đa khi ở trong thành */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Config")
	int32 MaxFlaskCharges = 5;
};

/**
 * FPAZoneModel
 *
 * Pure data model quản lý:
 * 1. Mạng lưới Tòa Thành (Citadels / Safe Cities) theo 3 phân vùng.
 * 2. Vùng An Toàn Tuyệt Đối (Safe Zone): Cấm PvP, quái drop aggro không thể vào.
 * 3. Cơ chế Tự Động Lưu (Auto-Save): Kích hoạt tức thì khi người chơi bước vào Tòa Thành.
 * 4. Cơ chế Đăng Nhập Lại (Relog Return): Khi thoát game ở bất kỳ đâu ngoài hoang dã,
 *    lúc đăng nhập vào lại người chơi luôn xuất hiện tại Tòa Thành ghé thăm gần nhất.
 * 5. Dịch chuyển nhanh giữa các Tòa Thành (Fast Travel 2.0s cast).
 * 6. Ranh giới xích quái AI Leash 2500cm.
 * 100% headless testable.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAZoneModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Model")
	TArray<FPACitadelNode> Citadels;

	/** Tòa thành người chơi bước vào gần nhất (Last Visited Citadel) */
	UPROPERTY(BlueprintReadOnly, Category = "Zone|Model")
	FName LastVisitedCitadelId;

	/** Bản ghi tự động lưu gần nhất */
	UPROPERTY(BlueprintReadOnly, Category = "Zone|Model")
	FPAAutoSaveRecord LastAutoSave;

	UPROPERTY(BlueprintReadOnly, Category = "Zone|Model")
	EPACitadelState CurrentState = EPACitadelState::Undiscovered;

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

	void RegisterCitadel(FName Id, const FString& DisplayName, EPAZoneTier Tier, const FVector& Location, float SafeRadius = 5000.0f)
	{
		FPACitadelNode Node;
		Node.CitadelId = Id;
		Node.CitadelDisplayName = DisplayName;
		Node.ZoneTier = Tier;
		Node.WorldLocation = Location;
		Node.SafeZoneRadius = (SafeRadius > 0.0f) ? SafeRadius : Config.DefaultCitadelRadius;
		Node.bIsDiscovered = false;
		Node.bIsLastVisited = false;
		Citadels.Add(Node);
	}

	// ===========================================================
	// Citadel Safe Zone & Auto-Save Actions
	// ===========================================================

	/**
	 * Người chơi bước vào Tòa Thành:
	 * - Đánh dấu Tòa Thành là Discovered.
	 * - Cập nhật LastVisitedCitadelId thành Tòa Thành này.
	 * - Kích hoạt Auto-Save tức thì lưu vị trí và trạng thái người chơi.
	 * - Hồi phục trọn vẹn 100% HP, MP, Stamina và đầy 5/5 Flasks.
	 * @return true nếu người chơi đang ở trong phạm vi một Tòa Thành và kích hoạt Auto-Save thành công.
	 */
	bool EnterCitadel(
		const FString& PlayerId,
		const FVector& PlayerLocation,
		float CurrentSimTime,
		FPAAutoSaveRecord& OutSaveRecord,
		float& OutHP,
		float& OutMP,
		float& OutStamina,
		int32& OutFlasks)
	{
		FName FoundCitadelId = NAME_None;
		if (!IsInsideSafeZone(PlayerLocation, FoundCitadelId))
		{
			return false;
		}

		FPACitadelNode* Node = FindCitadel(FoundCitadelId);
		if (!Node)
		{
			return false;
		}

		// Đánh dấu đã khám phá và đặt làm thành vào gần nhất
		Node->bIsDiscovered = true;
		SetLastVisitedCitadel(FoundCitadelId);

		// Hồi phục toàn diện tài nguyên
		OutHP = 1.0f;
		OutMP = 1.0f;
		OutStamina = 1.0f;
		OutFlasks = Config.MaxFlaskCharges;

		// Kích hoạt Auto-Save
		OutSaveRecord.PlayerId = PlayerId;
		OutSaveRecord.LastVisitedCitadelId = FoundCitadelId;
		OutSaveRecord.SavedSpawnLocation = Node->WorldLocation;
		OutSaveRecord.AutoSaveTimestamp = CurrentSimTime;
		OutSaveRecord.bSavedSuccessfully = true;
		OutSaveRecord.SavedHPPercent = OutHP;
		OutSaveRecord.SavedMPPercent = OutMP;
		OutSaveRecord.SavedStaminaPercent = OutStamina;
		OutSaveRecord.SavedFlaskCharges = OutFlasks;

		LastAutoSave = OutSaveRecord;
		CurrentState = EPACitadelState::Discovered;
		return true;
	}

	void SetLastVisitedCitadel(FName CitadelId)
	{
		LastVisitedCitadelId = CitadelId;
		for (FPACitadelNode& Node : Citadels)
		{
			Node.bIsLastVisited = (Node.CitadelId == CitadelId);
		}
	}

	/**
	 * Xử lý khi người chơi thoát game và đăng nhập vào lại (Relog):
	 * Luôn đưa người chơi về Tòa Thành mà họ đã ghé thăm gần nhất.
	 * @return true nếu tìm thấy tọa độ tòa thành gần nhất; OutSpawnLocation là tọa độ xuất hiện an toàn.
	 */
	bool GetRelogSpawnTransform(const FString& InPlayerId, FVector& OutSpawnLocation, FName& OutCitadelId) const
	{
		// 1. Kiểm tra bản ghi AutoSave trước
		if (LastAutoSave.bSavedSuccessfully && !LastAutoSave.LastVisitedCitadelId.IsNone())
		{
			const FPACitadelNode* Node = FindCitadel(LastAutoSave.LastVisitedCitadelId);
			if (Node)
			{
				OutSpawnLocation = Node->WorldLocation;
				OutCitadelId = Node->CitadelId;
				return true;
			}
		}

		// 2. Tra cứu theo LastVisitedCitadelId hiện tại
		if (!LastVisitedCitadelId.IsNone())
		{
			const FPACitadelNode* Node = FindCitadel(LastVisitedCitadelId);
			if (Node)
			{
				OutSpawnLocation = Node->WorldLocation;
				OutCitadelId = Node->CitadelId;
				return true;
			}
		}

		// 3. Fallback: Nếu là tài khoản hoàn toàn mới chưa từng vào thành nào, đưa về Thành Tân Thủ đầu tiên (Tier 1)
		for (const FPACitadelNode& Node : Citadels)
		{
			if (Node.ZoneTier == EPAZoneTier::Tier1_VerdantFrontier)
			{
				OutSpawnLocation = Node.WorldLocation;
				OutCitadelId = Node.CitadelId;
				return true;
			}
		}

		// Fallback cuối cùng nếu chưa đăng ký thành nào
		if (Citadels.Num() > 0)
		{
			OutSpawnLocation = Citadels[0].WorldLocation;
			OutCitadelId = Citadels[0].CitadelId;
			return true;
		}

		OutSpawnLocation = FVector::ZeroVector;
		OutCitadelId = NAME_None;
		return false;
	}

	// ===========================================================
	// Fast Travel Between Citadels (2.0s Cast)
	// ===========================================================

	bool StartCitadelFastTravel(FName FromId, FName ToId, bool bInCombat)
	{
		if (bInCombat || bIsChannelingFastTravel)
		{
			return false;
		}

		const FPACitadelNode* From = FindCitadel(FromId);
		const FPACitadelNode* To = FindCitadel(ToId);

		// Cả 2 điểm phải tồn tại và thành đích phải từng được khám phá (Discovered)
		if (!From || !To || !To->bIsDiscovered)
		{
			return false;
		}

		bIsChannelingFastTravel = true;
		FastTravelDestinationId = ToId;
		FastTravelElapsed = 0.0f;
		CurrentState = EPACitadelState::FastTraveling;
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
			return true;
		}

		return false;
	}

	void CancelFastTravel()
	{
		bIsChannelingFastTravel = false;
		FastTravelDestinationId = NAME_None;
		FastTravelElapsed = 0.0f;
		CurrentState = EPACitadelState::Discovered;
	}

	void CompleteFastTravel()
	{
		bIsChannelingFastTravel = false;
		FastTravelElapsed = 0.0f;
		CurrentState = EPACitadelState::Discovered;
		SetLastVisitedCitadel(FastTravelDestinationId);
	}

	// ===========================================================
	// Safe Zone & AI Leash Boundary Math
	// ===========================================================

	/**
	 * Kiểm tra xem vị trí có nằm trong Vùng An Toàn (Safe Zone) của bất kỳ Tòa Thành nào không.
	 * Khi ở trong Safe Zone: cấm PvP, quái bị xóa aggro và không thể bước vào.
	 */
	bool IsInsideSafeZone(const FVector& Location, FName& OutCitadelId) const
	{
		for (const FPACitadelNode& Node : Citadels)
		{
			const float RadiusSq = FMath::Square(Node.SafeZoneRadius);
			const float DistSq = FVector::DistSquared2D(Location, Node.WorldLocation);
			if (DistSq <= RadiusSq)
			{
				OutCitadelId = Node.CitadelId;
				return true;
			}
		}

		OutCitadelId = NAME_None;
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

	FPACitadelNode* FindCitadel(FName Id)
	{
		for (FPACitadelNode& Node : Citadels)
		{
			if (Node.CitadelId == Id)
			{
				return &Node;
			}
		}
		return nullptr;
	}

	const FPACitadelNode* FindCitadel(FName Id) const
	{
		for (const FPACitadelNode& Node : Citadels)
		{
			if (Node.CitadelId == Id)
			{
				return &Node;
			}
		}
		return nullptr;
	}
};
