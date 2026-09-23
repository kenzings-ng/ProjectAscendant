// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "PACurrencyTypes.generated.h"

class UPACurrencyComponent;

/**
 * EPACurrencyType
 *
 * Hai loại tiền tệ cốt lõi của Project Ascendant theo GDD (merchant-economy.md):
 * - Gold: Vàng giao thương vĩ mô (chợ, thợ rèn, sửa đồ, phạt chết 50%). Giới hạn: 9,999,999.
 * - Ash Shards: Tàn Trang linh hồn (boss drops, nâng cấp kỹ năng, thần binh). Giới hạn: 99,999.
 */
UENUM(BlueprintType)
enum class EPACurrencyType : uint8
{
	Gold        = 0 UMETA(DisplayName = "Gold / Vàng"),
	AshShards   = 1 UMETA(DisplayName = "Ash Shards / Tàn Trang")
};

/**
 * Giới hạn dung tích lưu trữ tiền tệ tối đa (merchant-economy.md §Tuning Knobs).
 */
namespace PACurrencyLimits
{
	inline constexpr int64 MaxGold = 9999999LL;
	inline constexpr int64 MaxAshShards = 99999LL;

	inline int64 GetMaxLimit(EPACurrencyType Type)
	{
		switch (Type)
		{
		case EPACurrencyType::Gold:
			return MaxGold;
		case EPACurrencyType::AshShards:
			return MaxAshShards;
		default:
			return 0LL;
		}
	}
}

/**
 * EPACurrencyTransactionError
 *
 * Mã lỗi khi thực hiện giao dịch tiền tệ trên Dedicated Server.
 */
UENUM(BlueprintType)
enum class EPACurrencyTransactionError : uint8
{
	None                 = 0 UMETA(DisplayName = "None / Success"),
	InsufficientFunds    = 1 UMETA(DisplayName = "Insufficient Funds"),
	ExceedsMaxCapacity   = 2 UMETA(DisplayName = "Exceeds Max Capacity"),
	InvalidAmount        = 3 UMETA(DisplayName = "Invalid Amount (<= 0)"),
	TargetNotFound       = 4 UMETA(DisplayName = "Target Wallet Not Found"),
	RateLimitExceeded    = 5 UMETA(DisplayName = "Transaction Rate Limit Exceeded"),
	ServerRejected       = 6 UMETA(DisplayName = "Server Rejected")
};

/**
 * FPACurrencyEntry
 *
 * Mục lưu trữ từng loại tiền tệ trong FastArray phục vụ delta replication (ADR-0001, ADR-0003).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACurrencyEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Economy|Currency")
	EPACurrencyType CurrencyType = EPACurrencyType::Gold;

	UPROPERTY(BlueprintReadOnly, Category = "Economy|Currency")
	int64 Amount = 0;

	FPACurrencyEntry()
		: CurrencyType(EPACurrencyType::Gold)
		, Amount(0)
	{
	}

	FPACurrencyEntry(EPACurrencyType InType, int64 InAmount)
		: CurrencyType(InType)
		, Amount(InAmount)
	{
	}

	void PreReplicatedRemove(const struct FPACurrencyList& InArraySerializer);
	void PostReplicatedAdd(const struct FPACurrencyList& InArraySerializer);
	void PostReplicatedChange(const struct FPACurrencyList& InArraySerializer);
};

/**
 * FPACurrencyList
 *
 * Bộ điều phối FastArray delta replication cho ví tiền tệ (ADR-0003, AC-1).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPACurrencyList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPACurrencyEntry> Entries;

	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UPACurrencyComponent> OwnerComponent = nullptr;

	FPACurrencyList()
		: OwnerComponent(nullptr)
	{
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FPACurrencyEntry, FPACurrencyList>(Entries, DeltaParms, *this);
	}

	void MarkItemDirty(FPACurrencyEntry& Item)
	{
		FFastArraySerializer::MarkItemDirty(Item);
	}

	void MarkArrayDirty()
	{
		FFastArraySerializer::MarkArrayDirty();
	}
};

template<>
struct TStructOpsTypeTraits<FPACurrencyList> : public TStructOpsTypeTraitsBase2<FPACurrencyList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
