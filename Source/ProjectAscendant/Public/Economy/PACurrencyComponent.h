// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Economy/PACurrencyTypes.h"
#include "PACurrencyComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnCurrencyBalanceChanged, EPACurrencyType, Type, int64, NewBalance, int64, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnCurrencyTransactionFailed, EPACurrencyType, Type, EPACurrencyTransactionError, ErrorCode);

/**
 * UPACurrencyComponent
 *
 * Component quản lý ví song tiền tệ (Vàng & Tàn Trang) theo chuẩn Server-Authoritative (Story econ-001, ADR-0001, ADR-0003).
 *
 * Tiêu chuẩn nghiệm thu Story econ-001:
 * - AC-1 (Dual Currency FastArray Replication): Quản lý Gold (max 9,999,999) và Ash Shards (max 99,999) với delta serialization.
 * - AC-2 (Atomic Transaction RPCs & Anti-Duping): Server RPCs thực hiện giao dịch nguyên tử, chống duping, kiểm tra số dư và giới hạn.
 * - AC-3 (PvE Death Penalty): Khi tử trận trong PvE, khấu trừ chính xác 50% số Gold hiện mang theo, bảo toàn 100% Ash Shards.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPACurrencyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPACurrencyComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// -------------------------------------------------------------------------
	// Currency Balance Queries (AC-1)
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Economy")
	int64 GetGold() const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Economy")
	int64 GetAshShards() const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Economy")
	int64 GetCurrency(EPACurrencyType Type) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Economy")
	bool HasSufficientCurrency(EPACurrencyType Type, int64 RequiredAmount) const;

	// -------------------------------------------------------------------------
	// Server-Authoritative Mutators & Atomic Operations (AC-2)
	// -------------------------------------------------------------------------

	/**
	 * Thêm tiền tệ vào ví (Server-Only). Tự động kẹp trong giới hạn MaxLimit.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Economy")
	bool AddCurrency(EPACurrencyType Type, int64 Amount, EPACurrencyTransactionError& OutError);

	/**
	 * Trừ tiền tệ từ ví (Server-Only). Thất bại nếu số dư không đủ hoặc Amount <= 0.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Economy")
	bool DeductCurrency(EPACurrencyType Type, int64 Amount, EPACurrencyTransactionError& OutError);

	/**
	 * Chuyển tiền tệ nguyên tử sang ví khác (Server-Only).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Economy")
	bool TransferCurrencyTo(UPACurrencyComponent* TargetWallet, EPACurrencyType Type, int64 Amount, EPACurrencyTransactionError& OutError);

	/**
	 * Xử lý hình phạt tử trận PvE (AC-3): Khấu trừ 50% Gold, OutGoldLost là lượng rơi ra đất.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Economy")
	bool HandlePvEDeathPenalty(int64& OutGoldLost);

	// -------------------------------------------------------------------------
	// Client RPCs / Server RPCs
	// -------------------------------------------------------------------------

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TransferCurrency(UPACurrencyComponent* TargetWallet, EPACurrencyType Type, int64 Amount);

	// -------------------------------------------------------------------------
	// Delegates & Callbacks
	// -------------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Economy")
	FPAOnCurrencyBalanceChanged OnCurrencyBalanceChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Economy")
	FPAOnCurrencyTransactionFailed OnCurrencyTransactionFailed;

	void OnReplicatedCurrencyChanged(EPACurrencyType Type, int64 OldAmount, int64 NewAmount);

protected:
	UPROPERTY(Replicated)
	FPACurrencyList Currencies;

private:
	void InitializeDefaultCurrencies();
	FPACurrencyEntry* FindCurrencyEntryMutable(EPACurrencyType Type);
	const FPACurrencyEntry* FindCurrencyEntry(EPACurrencyType Type) const;
};
