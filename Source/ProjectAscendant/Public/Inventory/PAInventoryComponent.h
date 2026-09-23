// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/PAInventoryTypes.h"
#include "PAInventoryComponent.generated.h"

class UItemStaticDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnInventorySlotUpdated, int32, SlotIndex, int32, NewQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnInventoryCapacityChanged, int32, NewCapacity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnItemMoved, int32, SourceSlot, int32, TargetSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnItemSplit, int32, SourceSlot, int32, TargetSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnOverflowStashItemAdded, const FPAInventoryItemEntry&, Entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPAOnInventoryTransactionRejected, int64, TransactionID, EPAInventoryTransactionError, ErrorCode);

/**
 * UPAInventoryComponent
 *
 * Component quản lý túi đồ lưới 30-60 ô chuẩn Server-Authoritative (Stories inv-001 & inv-002, ADR-0003).
 *
 * Tiêu chuẩn nghiệm thu Story inv-002:
 * - AC-1 (Atomic Move & Swap Validation): Di chuyển / hoán đổi nguyên tử (atomic) trên Server, kiểm tra UID chống duping.
 * - AC-2 (Split Stack Validation): Tách chồng đồ trên Server, kiểm tra ô đích rỗng và lượng tách hợp lệ.
 * - AC-3 (QoL Item Lock & Junk Flags): Khóa vật phẩm (bIsLocked) chống bán/vứt/hủy nhầm; Đánh dấu rác (bIsJunk) để bán nhanh.
 * - AC-4 (Overflow Stash Routing): Túi đầy 30/30 -> Vật phẩm Tier Rare trở lên rơi từ Boss tự động chuyển vào Hòm Đệm (20 ô).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAInventoryComponent();

	virtual void BeginPlay() override;

	// -------------------------------------------------------------------------
	// Capacity & Expansion (Story 001 / AC-1)
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Inventory")
	int32 GetCurrentMaxSlots() const { return CurrentMaxSlots; }

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool UnlockMoreSlots(int32 AdditionalSlots = 10);

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Inventory")
	bool IsValidSlotIndex(int32 SlotIndex) const { return SlotIndex >= 0 && SlotIndex < CurrentMaxSlots; }

	// -------------------------------------------------------------------------
	// Item Manipulation API (AC-3, Server-Authoritative)
	// -------------------------------------------------------------------------

	/**
	 * Thêm vật phẩm vào túi đồ (tự động cộng dồn theo AC-3 và tràn sang ô mới nếu vượt stack limit).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool TryAddItem(UItemStaticDataAsset* ItemData, int32 Quantity, int32& OutRemainingQuantity);

	/**
	 * Gán hoặc cập nhật vật phẩm vào một ô chỉ định cụ thể.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool AddItemToSlot(int32 SlotIndex, UItemStaticDataAsset* ItemData, int32 Quantity, const FPAItemInstanceData& DynamicData = FPAItemInstanceData(), const FGuid& InItemUID = FGuid());

	/**
	 * Trừ bớt hoặc dọn sạch vật phẩm tại một ô chỉ định.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool RemoveItemFromSlot(int32 SlotIndex, int32 QuantityToRemove = 1);

	/**
	 * Truy vấn thông tin vật phẩm tại một ô chỉ định (Raw C++ pointer, không expose trực tiếp Blueprint).
	 */
	const FPAInventoryItemEntry* GetItemAtSlot(int32 SlotIndex) const;

	/**
	 * Tìm ô trống đầu tiên có chỉ số nhỏ nhất trong phạm vi ô đã mở khóa.
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Inventory")
	int32 FindFirstEmptySlot() const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Inventory")
	bool IsSlotOccupied(int32 SlotIndex) const { return GetItemAtSlot(SlotIndex) != nullptr; }

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Inventory")
	int32 FindSlotByItemUID(const FGuid& ItemUID) const;

	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Inventory")
	int32 GetItemCount(FName ItemId) const;

	/**
	 * Tiêu hao một lượng vật phẩm chỉ định theo ID trên các ô trong túi đồ (Server-Authoritative).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool ConsumeItemQuantity(FName ItemId, int32 QuantityToConsume);

	/**
	 * Cập nhật dữ liệu động (DynamicData) của vật phẩm tại ô chỉ định (Server-Authoritative).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool UpdateItemDynamicData(int32 SlotIndex, const FPAItemInstanceData& NewDynamicData);

	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	void ClearInventory();

	// -------------------------------------------------------------------------
	// Server-Authoritative Transaction RPCs & Duplication Safeguards (Story 002)
	// -------------------------------------------------------------------------

	/**
	 * AC-1: Server RPC di chuyển hoặc hoán đổi ô đồ nguyên tử (Atomic Move & Swap).
	 * Kiểm tra ExpectedItemUID và TransactionID để ngăn chặn nhân bản vật phẩm (Duping) khi spam gói tin.
	 */
	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Inventory")
	void Server_MoveItem(int32 SourceSlot, int32 TargetSlot, FGuid ExpectedItemUID, int64 TransactionID);

	/**
	 * Logic thực thi Move/Swap trên Dedicated Server (hỗ trợ kiểm thử đơn vị độc lập).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool MoveItem(int32 SourceSlot, int32 TargetSlot, FGuid ExpectedItemUID = FGuid(), int64 TransactionID = 0);

	/**
	 * AC-2: Server RPC tách chồng đồ (Split Stack).
	 * Yêu cầu: SplitAmount < SourceStack và TargetSlot phải là ô trống.
	 */
	UFUNCTION(Server, Reliable, WithValidation, Category = "ProjectAscendant|Inventory")
	void Server_SplitStack(int32 SourceSlot, int32 SplitAmount, int32 TargetSlot, int64 TransactionID);

	/**
	 * Logic thực thi tách chồng đồ trên Server (hỗ trợ kiểm thử đơn vị).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool SplitStack(int32 SourceSlot, int32 SplitAmount, int32 TargetSlot, int64 TransactionID = 0);

	/**
	 * Client RPC thông báo giao dịch thất bại và kích hoạt rollback UI trên client.
	 */
	UFUNCTION(Client, Reliable, Category = "ProjectAscendant|Inventory")
	void Client_NotifyTransactionRejected(int64 TransactionID, EPAInventoryTransactionError ErrorCode);

	// -------------------------------------------------------------------------
	// QoL Item Lock & Junk Flags (Story 002 / AC-3)
	// -------------------------------------------------------------------------

	/**
	 * Bật/tắt trạng thái khóa vật phẩm (bIsLocked).
	 * Vật phẩm bị khóa không thể bị vứt, bán cho thương nhân hay phân rã.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool SetItemLocked(int32 SlotIndex, bool bLocked);

	/**
	 * Đảo trạng thái rác (bIsJunk) của vật phẩm (phục vụ phím tắt [J]).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool ToggleItemJunk(int32 SlotIndex);

	/**
	 * Kiểm tra xem vật phẩm tại ô này có được phép bán hoặc vứt bỏ hay không.
	 * Trả về False nếu vật phẩm đang bị khóa (bIsLocked == true).
	 */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Inventory")
	bool CanDropOrSellItem(int32 SlotIndex) const;

	// -------------------------------------------------------------------------
	// Overflow Stash Routing (Story 002 / AC-4)
	// -------------------------------------------------------------------------

	/**
	 * Chuyển vật phẩm quý (Tier Rare trở lên) vào Hòm Đệm Lửa Trại (20 ô) khi túi đầy (AC-4).
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool RouteToOverflowStash(UItemStaticDataAsset* ItemData, int32 Quantity, const FPAItemInstanceData& DynamicData = FPAItemInstanceData());

	/** Trả về số lượng vật phẩm đang lưu trong Hòm Đệm (tối đa 20). */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Inventory")
	int32 GetOverflowStashCount() const { return OverflowStash.Num(); }

	/** Lấy danh sách vật phẩm trong Hòm Đệm. */
	UFUNCTION(BlueprintPure, Category = "ProjectAscendant|Inventory")
	const TArray<FPAInventoryItemEntry>& GetOverflowStash() const { return OverflowStash; }

	/**
	 * Nhận vật phẩm từ Hòm Đệm đưa vào túi đồ khi đã có ô trống.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|Inventory")
	bool ClaimOverflowStashItem(int32 StashIndex);

	// -------------------------------------------------------------------------
	// FastArray Replication Callbacks (Story 001 / AC-4)
	// -------------------------------------------------------------------------
	void HandleItemAdded(const FPAInventoryItemEntry& Entry);
	void HandleItemChanged(const FPAInventoryItemEntry& Entry);
	void HandleItemRemoved(const FPAInventoryItemEntry& Entry);

public:
	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Inventory")
	FPAOnInventorySlotUpdated OnInventorySlotUpdated;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Inventory")
	FPAOnInventoryCapacityChanged OnInventoryCapacityChanged;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Inventory")
	FPAOnItemMoved OnItemMoved;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Inventory")
	FPAOnItemSplit OnItemSplit;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Inventory")
	FPAOnOverflowStashItemAdded OnOverflowStashItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|Inventory")
	FPAOnInventoryTransactionRejected OnTransactionRejected;

protected:
	static constexpr int32 kBaseInventorySlots = 30;
	static constexpr int32 kMaxInventorySlots = 60;
	static constexpr int32 kSlotExpansionStep = 10;
	static constexpr int32 kMaxOverflowStashSlots = 20; // AC-4: Giới hạn 20 ô hòm đệm

	UPROPERTY(Replicated)
	FPAInventoryItemList InventoryList;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentMaxSlots, BlueprintReadOnly, Category = "ProjectAscendant|Inventory")
	int32 CurrentMaxSlots = kBaseInventorySlots;

	/** Danh sách Hòm Đệm (Overflow Stash) lưu trữ đồ quý khi ba lô đầy (AC-4) */
	UPROPERTY(Replicated)
	TArray<FPAInventoryItemEntry> OverflowStash;

	/** Theo dõi ID giao dịch gần nhất được xử lý để ngăn chặn Replay Attacks */
	UPROPERTY()
	int64 LastProcessedTransactionID = 0;

	UFUNCTION()
	virtual void OnRep_CurrentMaxSlots();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	FPAInventoryItemEntry* FindMutableEntryAtSlot(int32 SlotIndex);
};
