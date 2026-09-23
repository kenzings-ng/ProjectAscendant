// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "PAInventoryTypes.generated.h"

class UItemStaticDataAsset;
class UPAInventoryComponent;
struct FPAEquipmentItemList;

/**
 * EPAItemRarity
 *
 * 5 Tier cấp độ hiếm theo GDD (inventory-system.md):
 * - Tier 1: Common (Trắng)
 * - Tier 2: Uncommon (Xanh lục)
 * - Tier 3: Rare (Xanh lam)
 * - Tier 4: Epic (Tím)
 * - Tier 5: Legendary (Cam / Hoàng kim)
 */
UENUM(BlueprintType)
enum class EPAItemRarity : uint8
{
	None         = 0 UMETA(DisplayName = "None / Unassigned"),
	Common       = 1 UMETA(DisplayName = "Tier 1: Common"),
	Uncommon     = 2 UMETA(DisplayName = "Tier 2: Uncommon"),
	Rare         = 3 UMETA(DisplayName = "Tier 3: Rare"),
	Epic         = 4 UMETA(DisplayName = "Tier 4: Epic"),
	Legendary    = 5 UMETA(DisplayName = "Tier 5: Legendary")
};

/**
 * EPAItemCategory
 *
 * Phân loại vật phẩm quy định giới hạn chồng đồ (AC-3):
 * - Equipment: max stack 1
 * - SkillBook: max stack 1
 * - Consumable: max stack 20
 * - Material: max stack 999
 * - Quest: max stack 1
 */
UENUM(BlueprintType)
enum class EPAItemCategory : uint8
{
	Equipment     UMETA(DisplayName = "Equipment (Stack 1)"),
	SkillBook     UMETA(DisplayName = "Skill Book (Stack 1)"),
	Consumable    UMETA(DisplayName = "Consumable (Stack 20)"),
	Material      UMETA(DisplayName = "Material / Shard (Stack 999)"),
	Quest         UMETA(DisplayName = "Quest Item (Stack 1)")
};

/**
 * EPAEquipmentSlot
 *
 * 6 vị trí ô trang bị Paperdoll theo chuẩn GDD inventory-system.md (AC-1):
 * 1. Mainhand Weapon
 * 2. Offhand / Shield
 * 3. Body Armor
 * 4. Amulet
 * 5. Ring Slot 1
 * 6. Ring Slot 2
 */
UENUM(BlueprintType)
enum class EPAEquipmentSlot : uint8
{
	None        = 0 UMETA(DisplayName = "None / Unassigned"),
	Mainhand    = 1 UMETA(DisplayName = "Mainhand Weapon"),
	Offhand     = 2 UMETA(DisplayName = "Offhand / Shield"),
	BodyArmor   = 3 UMETA(DisplayName = "Body Armor (Chest)"),
	Amulet      = 4 UMETA(DisplayName = "Amulet"),
	Ring1       = 5 UMETA(DisplayName = "Ring Slot 1"),
	Ring2       = 6 UMETA(DisplayName = "Ring Slot 2")
};

namespace PAEquipmentSlots
{
	inline constexpr EPAEquipmentSlot Chest = EPAEquipmentSlot::BodyArmor;
	inline constexpr EPAEquipmentSlot MainHand = EPAEquipmentSlot::Mainhand;
}

/**
 * EPAQuickbarSlot
 *
 * 4 vị trí ô phím tắt Quickbar 1–4 cho vật phẩm tiêu hao (GDD inventory-system.md §3.3).
 */
UENUM(BlueprintType)
enum class EPAQuickbarSlot : uint8
{
	None        = 0 UMETA(DisplayName = "None / Unassigned"),
	Quickbar_1  = 1 UMETA(DisplayName = "Quickbar Slot 1 (Key 1)"),
	Quickbar_2  = 2 UMETA(DisplayName = "Quickbar Slot 2 (Key 2)"),
	Quickbar_3  = 3 UMETA(DisplayName = "Quickbar Slot 3 (Key 3)"),
	Quickbar_4  = 4 UMETA(DisplayName = "Quickbar Slot 4 (Key 4)")
};

/**
 * EPAItemUseError
 *
 * Mã lỗi khi sử dụng vật phẩm / Sách Kỹ Năng / Bình Tiêu Hao (AC-3, AC-4).
 */
UENUM(BlueprintType)
enum class EPAItemUseError : uint8
{
	None                  = 0 UMETA(DisplayName = "None"),
	NotInCombatRequired   = 1 UMETA(DisplayName = "Cannot Read in Combat"),
	ClassMismatch         = 2 UMETA(DisplayName = "Class Mismatch"),
	InvalidSlot           = 3 UMETA(DisplayName = "Invalid Equipment Slot"),
	ItemNotFound          = 4 UMETA(DisplayName = "Item Not Found"),
	NotConsumable         = 5 UMETA(DisplayName = "Not a Consumable"),
	NotSkillBook          = 6 UMETA(DisplayName = "Not a Skill Book"),
	AlreadyDrinking       = 7 UMETA(DisplayName = "Already Drinking Potion"),
	EmptyQuickbarSlot     = 8 UMETA(DisplayName = "Quickbar Slot Is Empty")
};

/**
 * EPAInventoryTransactionError
 *
 * Mã lỗi phản hồi khi giao dịch ô đồ trên Dedicated Server bị từ chối (Story 002).
 */
UENUM(BlueprintType)
enum class EPAInventoryTransactionError : uint8
{
	None                  = 0 UMETA(DisplayName = "None"),
	InvalidSourceSlot     = 1 UMETA(DisplayName = "Invalid Source Slot"),
	InvalidTargetSlot     = 2 UMETA(DisplayName = "Invalid Target Slot"),
	ItemNotFound          = 3 UMETA(DisplayName = "Item Not Found"),
	UIDMismatch           = 4 UMETA(DisplayName = "UID Mismatch (Anti-Duping)"),
	ItemLocked            = 5 UMETA(DisplayName = "Item Is Locked"),
	TargetSlotOccupied    = 6 UMETA(DisplayName = "Target Slot Occupied"),
	InvalidSplitAmount    = 7 UMETA(DisplayName = "Invalid Split Amount"),
	InventoryFull         = 8 UMETA(DisplayName = "Inventory Full"),
	StashFull             = 9 UMETA(DisplayName = "Overflow Stash Full"),
	ReplayDetected        = 10 UMETA(DisplayName = "Duplicate Packet Replay"),
	ServerRejected        = 11 UMETA(DisplayName = "Server Rejected")
};

/**
 * FPAItemInstanceData
 *
 * Dữ liệu động của từng phiên bản vật phẩm cụ thể (độ bền, cường hóa, ngọc khảm).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAItemInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Instance")
	float CurrentDurability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Instance", meta = (ClampMin = "0", ClampMax = "10"))
	int32 EnhancementLevel = 0; // +0 đến +10

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Instance")
	TArray<FName> SocketedGemIds;

	/** Khóa vật phẩm chống bán, vứt hoặc phân rã nhầm (AC-3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Instance")
	bool bIsLocked = false;

	/** Đánh dấu rác để thanh lý nhanh tại thương nhân (AC-3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Instance")
	bool bIsJunk = false;

	FPAItemInstanceData()
		: CurrentDurability(100.0f)
		, EnhancementLevel(0)
		, bIsLocked(false)
		, bIsJunk(false)
	{
	}
};

/**
 * FPAInventoryItemEntry
 *
 * Cấu trúc đại diện cho 1 ô vật phẩm trong túi đồ dạng lưới, kế thừa FFastArraySerializerItem
 * để tối ưu hóa truyền tin Delta qua kênh mạng Iris (ADR-0003, AC-4).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Entry")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Entry")
	FGuid ItemInstanceUID;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Entry")
	FName ItemDefId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Entry")
	TObjectPtr<UItemStaticDataAsset> StaticData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Entry")
	int32 StackCount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Entry")
	FPAItemInstanceData DynamicData;

	FPAInventoryItemEntry()
		: SlotIndex(INDEX_NONE)
		, ItemInstanceUID(FGuid())
		, ItemDefId(NAME_None)
		, StaticData(nullptr)
		, StackCount(1)
	{
	}

	FPAInventoryItemEntry(int32 InSlotIndex, FName InItemDefId, UItemStaticDataAsset* InStaticData, int32 InStackCount = 1, const FGuid& InUID = FGuid())
		: SlotIndex(InSlotIndex)
		, ItemInstanceUID(InUID.IsValid() ? InUID : FGuid::NewGuid())
		, ItemDefId(InItemDefId)
		, StaticData(InStaticData)
		, StackCount(InStackCount)
	{
	}

	void PreReplicatedRemove(const struct FPAInventoryItemList& InArraySerializer);
	void PostReplicatedAdd(const struct FPAInventoryItemList& InArraySerializer);
	void PostReplicatedChange(const struct FPAInventoryItemList& InArraySerializer);

	void PreReplicatedRemove(const struct FPAEquipmentItemList& InArraySerializer) {}
	void PostReplicatedAdd(const struct FPAEquipmentItemList& InArraySerializer) {}
	void PostReplicatedChange(const struct FPAEquipmentItemList& InArraySerializer) {}
};

/**
 * FPAInventoryItemList
 *
 * Bộ điều phối mảng nén mạng FastArray cho toàn bộ 30-60 ô túi đồ (ADR-0003, AC-4).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAInventoryItemList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPAInventoryItemEntry> Items;

	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UPAInventoryComponent> OwnerComponent = nullptr;

	FPAInventoryItemList()
		: OwnerComponent(nullptr)
	{
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FPAInventoryItemEntry, FPAInventoryItemList>(Items, DeltaParms, *this);
	}

	void MarkItemDirty(FPAInventoryItemEntry& Item)
	{
		FFastArraySerializer::MarkItemDirty(Item);
	}

	void MarkArrayDirty()
	{
		FFastArraySerializer::MarkArrayDirty();
	}
};

template<>
struct TStructOpsTypeTraits<FPAInventoryItemList> : public TStructOpsTypeTraitsBase2<FPAInventoryItemList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

class UPAEquipmentComponent;

/**
 * FPAEquipmentItemList
 *
 * Bộ điều phối mảng nén mạng FastArray cho 6 ô trang bị Paperdoll (ADR-0003, AC-1).
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPAEquipmentItemList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPAInventoryItemEntry> Items;

	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UPAEquipmentComponent> OwnerComponent = nullptr;

	FPAEquipmentItemList()
		: OwnerComponent(nullptr)
	{
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FPAInventoryItemEntry, FPAEquipmentItemList>(Items, DeltaParms, *this);
	}

	void MarkItemDirty(FPAInventoryItemEntry& Item)
	{
		FFastArraySerializer::MarkItemDirty(Item);
	}

	void MarkArrayDirty()
	{
		FFastArraySerializer::MarkArrayDirty();
	}
};

template<>
struct TStructOpsTypeTraits<FPAEquipmentItemList> : public TStructOpsTypeTraitsBase2<FPAEquipmentItemList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

