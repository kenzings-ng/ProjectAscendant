// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/PAPaperdollTypes.h"
#include "Inventory/PAInventoryTypes.h"
#include "PAPaperdollComponent.generated.h"

class APABaseCharacter;
class UPAEquipmentComponent;
class UPaperFlipbookComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FPAOnPaperdollVisualChanged, EPAPaperdollLayer, Layer, FName, ItemId, FName, VisualAssetId, bool, bIsVisible);

/**
 * UPAPaperdollComponent
 *
 * Quản lý trực quan hóa trang bị đa tầng (Modular Paperdoll Visualizer) cho nhân vật:
 * - Khởi đầu với trang phục vải thô tân thủ (Starter Linen Cloth).
 * - Lắng nghe sự kiện OnItemEquipped / OnItemUnequipped từ UPAEquipmentComponent.
 * - Khi người chơi click mặc đồ (Áo giáp sắt, Đồ da thợ săn, Áo choàng pháp sư, Vũ khí, Khiên),
 *   lập tức cập nhật tầng hiển thị Sprite tương ứng (EPAPaperdollLayer).
 * - Khi tháo đồ, tự động khôi phục về đồ vải ban đầu.
 * - Đồng bộ khung hình hoạt ảnh (Frame Synchronization) giữa các tầng flipbook với BaseSprite.
 * - Tương thích 100% headless testing thông qua FPAPaperdollModel.
 */
UCLASS(ClassGroup = (Character), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAPaperdollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAPaperdollComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Khởi tạo liên kết với UPAEquipmentComponent của chủ thể (Owner Actor).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void BindToEquipmentComponent(UPAEquipmentComponent* InEquipmentComponent);

	/**
	 * Mặc một món trang bị lên Paperdoll và cập nhật trực quan.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	bool EquipVisual(EPAPaperdollLayer Layer, FName ItemId, FName VisualAssetId);

	/**
	 * Tháo một món trang bị khỏi Paperdoll và đưa về trang phục mặc định.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	bool UnequipVisual(EPAPaperdollLayer Layer);

	/**
	 * Trả nhân vật về trang phục quần áo vải thô tân thủ.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void ResetToStarterCloth();

	/**
	 * Kiểm tra xem nhân vật hiện tại có đang mặc bộ đồ vải tân thủ hay không.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	bool IsWearingStarterCloth() const { return Model.IsWearingStarterCloth(); }

	/**
	 * Trả về ngoại hình trang phục tổng hợp hiện tại.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	const FPAPaperdollAppearance& GetAppearance() const { return Model.CurrentAppearance; }

	/**
	 * Trả về Model dữ liệu thuần (Pure Data Model) phục vụ unit test headless.
	 */
	const FPAPaperdollModel& GetModel() const { return Model; }
	FPAPaperdollModel& GetMutableModel() { return Model; }

	/**
	 * Mặc trang bị vào một trong 9 ô slot của Paperdoll (Story item-004).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	bool EquipSlot(EPAPaperdollSlot Slot, FName ItemId, FName VisualAssetId);

	/**
	 * Tháo trang bị khỏi một trong 9 ô slot của Paperdoll (Story item-004).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	bool UnequipSlot(EPAPaperdollSlot Slot);

	/**
	 * Đăng ký / Ánh xạ một Flipbook Component cho một trong 9 ô slot (item-004).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void RegisterSlotFlipbookComponent(EPAPaperdollSlot Slot, UPaperFlipbookComponent* FlipbookComp);

	/**
	 * Lấy Flipbook Component được gán cho slot chỉ định.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	UPaperFlipbookComponent* GetSlotFlipbookComponent(EPAPaperdollSlot Slot) const;

	/**
	 * Cập nhật Directional Sort Key / Translucent Sort Priority theo hướng ngắm 8 chiều:
	 * Tự động đảo priority giữa MainHand và OffHand khi mirror hướng Tây/Tây Nam/Tây Bắc.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void UpdateDirectionalSortKeys(EPAAimDirection8Way Direction);

	/**
	 * Khởi tạo tự động 9 sub-components cho 9 slot (gắn HandSocket_R/L, pivot chân 64,114).
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void Initialize9SlotSubcomponents();

	/**
	 * Lấy hướng ngắm hiện tại của Paperdoll.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	EPAAimDirection8Way GetCurrentOrientation() const { return CurrentOrientation; }

	/**
	 * Lấy mã tài nguyên hiển thị của tầng chỉ định.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	FName GetActiveVisualAssetId(EPAPaperdollLayer Layer) const { return Model.GetActiveVisualAssetId(Layer); }

	/**
	 * Đăng ký / Ánh xạ một Flipbook Component cho một Layer đồ họa.
	 */
	UFUNCTION(BlueprintCallable, Category = "Paperdoll")
	void RegisterLayerFlipbookComponent(EPAPaperdollLayer Layer, UPaperFlipbookComponent* FlipbookComp);

	/**
	 * Lấy Flipbook Component được gán cho Layer chỉ định.
	 */
	UFUNCTION(BlueprintPure, Category = "Paperdoll")
	UPaperFlipbookComponent* GetLayerFlipbookComponent(EPAPaperdollLayer Layer) const;

public:
	/** Sự kiện phát thanh khi ngoại hình một lớp trang bị thay đổi */
	UPROPERTY(BlueprintAssignable, Category = "Paperdoll|Events")
	FPAOnPaperdollVisualChanged OnPaperdollVisualChanged;

protected:
	/** Pure Data Model lưu trữ trạng thái hiển thị của các tầng Paperdoll */
	UPROPERTY(ReplicatedUsing = OnRep_Model)
	FPAPaperdollModel Model;

	UFUNCTION()
	virtual void OnRep_Model();

	/** Bắt sự kiện khi trang bị được mặc từ UPAEquipmentComponent */
	UFUNCTION()
	void HandleItemEquipped(EPAEquipmentSlot Slot, const FPAInventoryItemEntry& Item);

	/** Bắt sự kiện khi trang bị bị tháo từ UPAEquipmentComponent */
	UFUNCTION()
	void HandleItemUnequipped(EPAEquipmentSlot Slot, const FPAInventoryItemEntry& Item);

	/** Cập nhật trực quan và đồng bộ frame của các component đồ họa */
	void SynchronizeLayerSprites();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** Map tham chiếu đến các Flipbook Component tương ứng cho từng Layer (chỉ dùng trên Client/Standalone có render) */
	UPROPERTY(Transient)
	TMap<EPAPaperdollLayer, TObjectPtr<UPaperFlipbookComponent>> LayerComponents;

	/** Map tham chiếu đến 9 sub-component cho 9 slot (Story item-004) */
	UPROPERTY(Transient)
	TMap<EPAPaperdollSlot, TObjectPtr<UPaperFlipbookComponent>> SlotComponents;

	/** Hướng ngắm 8 chiều hiện tại */
	UPROPERTY(Transient)
	EPAAimDirection8Way CurrentOrientation = EPAAimDirection8Way::East;

	/** Con trỏ yếu đến EquipmentComponent đã liên kết */
	TWeakObjectPtr<UPAEquipmentComponent> BoundEquipmentComponent;
};
