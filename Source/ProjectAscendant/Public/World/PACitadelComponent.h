// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "World/PAZoneTypes.h"
#include "PACitadelComponent.generated.h"

/**
 * UPACitadelComponent
 *
 * ActorComponent quản lý Tòa Thành (Citadel / Stronghold / Safe City):
 * - Vùng An Toàn (Safe Zone): Cấm PvP, quái vật drop aggro không thể vào.
 * - Cơ chế Tự Động Lưu (Auto-Save): Kích hoạt ngay khi người chơi bước vào Tòa Thành.
 * - Lưu tiến trình và ghim Tòa Thành này làm điểm xuất hiện khi đăng nhập lại (Relog) hoặc tử trận.
 * - Hồi phục trọn vẹn 100% HP/Mana/Stamina và 5/5 bình Flasks.
 *
 * Tham chiếu GDD: design/gdd/zone-system.md §Citadel & Safe Zone System
 * ADR-0001: Server-authoritative logic
 */
UCLASS(ClassGroup = (World), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPACitadelComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPACitadelComponent();

	virtual void BeginPlay() override;

	// ===========================================================
	// Public API
	// ===========================================================

	UFUNCTION(BlueprintCallable, Category = "Zone|Citadel")
	void InitializeCitadel(FName InId, const FString& InDisplayName, EPAZoneTier InTier, float InSafeRadius = 5000.0f);

	/**
	 * Kích hoạt khi người chơi bước vào Tòa Thành:
	 * - Kích hoạt Auto-Save ngay lập tức.
	 * - Ghim Tòa Thành làm điểm xuất hiện khi thoát game/vào lại (Relog).
	 * - Hồi phục trọn vẹn tài nguyên (100% HP, MP, Stamina, đầy Flask).
	 */
	UFUNCTION(BlueprintCallable, Category = "Zone|Citadel")
	bool OnPlayerEnterCitadel(const FString& PlayerId, const FVector& PlayerLocation, FPAAutoSaveRecord& OutSaveRecord, float& OutHP, float& OutMP, float& OutStamina, int32& OutFlasks);

	UFUNCTION(BlueprintPure, Category = "Zone|Citadel")
	FName GetCitadelId() const { return CitadelData.CitadelId; }

	UFUNCTION(BlueprintPure, Category = "Zone|Citadel")
	FString GetCitadelDisplayName() const { return CitadelData.CitadelDisplayName; }

	UFUNCTION(BlueprintPure, Category = "Zone|Citadel")
	EPAZoneTier GetZoneTier() const { return CitadelData.ZoneTier; }

	UFUNCTION(BlueprintPure, Category = "Zone|Citadel")
	float GetSafeZoneRadius() const { return CitadelData.SafeZoneRadius; }

	UFUNCTION(BlueprintPure, Category = "Zone|Citadel")
	bool IsInsideSafeZone(const FVector& Pos) const;

	// ===========================================================
	// Delegates
	// ===========================================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCitadelEnteredAndSaved, FName, CitadelId, FString, PlayerId, const FPAAutoSaveRecord&, SaveRecord);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCitadelPlayerExited, FName, CitadelId, FString, PlayerId);

	UPROPERTY(BlueprintAssignable, Category = "Zone|Citadel|Events")
	FOnCitadelEnteredAndSaved OnCitadelEnteredAndSaved;

	UPROPERTY(BlueprintAssignable, Category = "Zone|Citadel|Events")
	FOnCitadelPlayerExited OnCitadelPlayerExited;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Citadel")
	FPACitadelNode CitadelData;

	FPAZoneModel InternalZoneModel;
};
