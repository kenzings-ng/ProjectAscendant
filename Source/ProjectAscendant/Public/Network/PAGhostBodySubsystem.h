// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PAGhostBodySubsystem.generated.h"

/**
 * FPAGhostBodyRecord
 *
 * Bản ghi trạng thái "Xác Ma" (Ghost Body) của người chơi khi bị ngắt kết nối trong lúc giao tranh.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAGhostBodyRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|GhostBody")
	FString PlayerId;

	/** Thời gian tồn tại còn lại của Xác Ma (khởi tạo 15.0s theo AC-4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|GhostBody")
	float RemainingGhostTime = 15.0f;

	/** Đánh dấu người chơi bị ngắt kết nối khi đang có tag InCombat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|GhostBody")
	bool bIsInCombat = false;

	/** Đánh dấu Xác Ma đã bị tiêu diệt bởi quái vật / người chơi khác */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|GhostBody")
	bool bIsDead = false;

	/** Đánh dấu hết 15s và chuẩn bị giải phóng an toàn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Netcode|GhostBody")
	bool bIsMarkedForDespawn = false;

	FPAGhostBodyRecord() = default;

	FPAGhostBodyRecord(const FString& InPlayerId, float InDuration, bool bInCombat)
		: PlayerId(InPlayerId)
		, RemainingGhostTime(InDuration)
		, bIsInCombat(bInCombat)
		, bIsDead(false)
		, bIsMarkedForDespawn(false)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAGhostBodyEvent, const FString&, PlayerId);

/**
 * UPAGhostBodySubsystem
 *
 * World Subsystem quản lý cơ chế chống rút dây mạng / thoát game trốn chết (Anti-Combat-Logging).
 * Theo tiêu chuẩn AC-4: Nhân vật bị rớt mạng trong giao tranh sẽ tồn tại 15.0s trên Dedicated Server,
 * vẫn nhận sát thương bình thường, cho phép Reconnect trong 15s hoặc chịu phạt tử vong.
 */
UCLASS()
class PROJECTASCENDANT_API UPAGhostBodySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Thời lượng tồn tại mặc định của Xác Ma trong combat: 15.0s */
	static constexpr float kDefaultCombatGhostDuration = 15.0f;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * AC-4: Đăng ký sự kiện người chơi ngắt kết nối.
	 * - Nếu bInCombat = false: Cho phép đăng xuất ngay lập tức (trả về false, không tạo Ghost Body).
	 * - Nếu bInCombat = true: Kích hoạt Xác Ma tồn tại 15.0s trên Dedicated Server (trả về true).
	 */
	UFUNCTION(BlueprintCallable, Category = "Netcode|GhostBody")
	bool RegisterDisconnect(const FString& PlayerId, bool bInCombat, float GhostDuration = 15.0f);

	/**
	 * AC-4: Xử lý sự kiện người chơi kết nối lại (Reconnect).
	 * Nếu người chơi kết nối lại trong vòng 15s: Hủy trạng thái Ghost Body, trả về true.
	 */
	UFUNCTION(BlueprintCallable, Category = "Netcode|GhostBody")
	bool HandleReconnect(const FString& PlayerId);

	/**
	 * Trừ thời gian tồn tại của Xác Ma theo DeltaTime.
	 * Khi hết 15s (RemainingTime <= 0), kích hoạt cờ bIsMarkedForDespawn và trả về true.
	 */
	UFUNCTION(BlueprintCallable, Category = "Netcode|GhostBody")
	bool TickGhostLifespan(const FString& PlayerId, float DeltaTime);

	/**
	 * Xử lý khi Xác Ma bị quái vật hoặc người chơi khác đánh trúng đòn chí mạng tử vong.
	 * Áp dụng hình phạt tử vong và đánh dấu bIsDead = true.
	 */
	UFUNCTION(BlueprintCallable, Category = "Netcode|GhostBody")
	bool ApplyLethalDamageToGhost(const FString& PlayerId);

	/** Kiểm tra người chơi có đang ở trạng thái Xác Ma hoạt động hay không */
	UFUNCTION(BlueprintPure, Category = "Netcode|GhostBody")
	bool IsGhostActive(const FString& PlayerId) const;

	/** Lấy thời gian còn lại của Xác Ma (giây) */
	UFUNCTION(BlueprintPure, Category = "Netcode|GhostBody")
	float GetRemainingGhostTime(const FString& PlayerId) const;

	/** Lấy tổng số lượng Xác Ma đang hoạt động trên Server */
	UFUNCTION(BlueprintPure, Category = "Netcode|GhostBody")
	int32 GetActiveGhostCount() const { return ActiveGhostBodies.Num(); }

	/** Dọn sạch toàn bộ Xác Ma */
	UFUNCTION(BlueprintCallable, Category = "Netcode|GhostBody")
	void ClearAllGhosts();

public:
	/** Sự kiện khi một Xác Ma được khởi tạo */
	UPROPERTY(BlueprintAssignable, Category = "Netcode|GhostBody|Events")
	FPAGhostBodyEvent OnGhostBodySpawned;

	/** Sự kiện khi một Xác Ma hết thời gian 15s và giải phóng an toàn */
	UPROPERTY(BlueprintAssignable, Category = "Netcode|GhostBody|Events")
	FPAGhostBodyEvent OnGhostBodyDespawned;

	/** Sự kiện khi Xác Ma bị đánh chết trong lúc rớt mạng */
	UPROPERTY(BlueprintAssignable, Category = "Netcode|GhostBody|Events")
	FPAGhostBodyEvent OnGhostBodyKilled;

private:
	/** Bảng tra cứu các Xác Ma đang hoạt động (PlayerId -> Record) */
	UPROPERTY(Transient)
	TMap<FString, FPAGhostBodyRecord> ActiveGhostBodies;
};
