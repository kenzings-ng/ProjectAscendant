// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/PACharacterSelectTypes.h"
#include "Input/Reply.h"
#include "PACharacterSelectWidget.generated.h"

class UPAAccountSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPAOnClassSelectionChanged, const FPACharacterClassInfo&, SelectedClassInfo);

/**
 * UPACharacterSelectWidget
 *
 * Widget quản lý màn hình chọn nhân vật (Character Selection Screen)
 * Hỗ trợ 3 chức nghiệp: Vanguard (Chiến Binh), Ranger (Du Hiệp), Arcanist (Thuật Sĩ).
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTASCENDANT_API UPACharacterSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Chọn một chức nghiệp cụ thể */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|UI|CharacterSelect")
	void SelectClass(EPACharacterClass InClass);

	/** Chuyển sang chức nghiệp kế tiếp */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|UI|CharacterSelect")
	void SelectNextClass();

	/** Chuyển về chức nghiệp trước đó */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|UI|CharacterSelect")
	void SelectPreviousClass();

	/** Lấy thông tin chức nghiệp đang được chọn */
	UFUNCTION(BlueprintPure, Category = "Ascendant|UI|CharacterSelect")
	FPACharacterClassInfo GetSelectedClassInfo() const;

	/** Lấy danh sách tất cả các chức nghiệp khả dụng */
	UFUNCTION(BlueprintPure, Category = "Ascendant|UI|CharacterSelect")
	TArray<FPACharacterClassInfo> GetAvailableClasses() const { return AvailableClasses; }

	/** Xác nhận lựa chọn và chuyển map vào thế giới game */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|UI|CharacterSelect")
	void ConfirmSelectionAndEnterWorld();

	/** Tên bản đồ thế giới cần nạp sau khi chọn nhân vật (Mặc định: Thị Trấn Tiền Trạm - L_VerdantFrontier_Outpost) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|UI|CharacterSelect")
	FName TargetGameWorldMapName = FName(TEXT("L_VerdantFrontier_Outpost"));

	/** Delegate phát khi người chơi chuyển đổi giữa các class */
	UPROPERTY(BlueprintAssignable, Category = "Ascendant|UI|CharacterSelect|Events")
	FPAOnClassSelectionChanged OnClassSelectionChanged;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	/** Sự kiện Blueprint để cập nhật UI hiển thị card, 3D model/sprite preview và chỉ số */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ascendant|UI|CharacterSelect")
	void OnClassSelectedBP(const FPACharacterClassInfo& SelectedClassInfo);

	/** Sự kiện Blueprint khi bắt đầu chuyển map */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ascendant|UI|CharacterSelect")
	void OnEnteringWorldBP(const FName& MapName);

	/** Lấy con trỏ Account Subsystem */
	UFUNCTION(BlueprintPure, Category = "Ascendant|UI|CharacterSelect")
	UPAAccountSubsystem* GetAccountSubsystem() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascendant|UI|CharacterSelect")
	EPACharacterClass CurrentlySelectedClass = EPACharacterClass::Vanguard;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ascendant|UI|CharacterSelect")
	TArray<FPACharacterClassInfo> AvailableClasses;

private:
	TSharedPtr<class STextBlock> SelectedClassNameText;
	TSharedPtr<class STextBlock> SelectedClassTaglineText;
	TSharedPtr<class STextBlock> SelectedClassLoreText;
	TSharedPtr<class STextBlock> SelectedClassRoleText;
	TSharedPtr<class STextBlock> SelectedClassWeaponText;
	TSharedPtr<class STextBlock> SelectedClassStatsText;
	TSharedPtr<class STextBlock> SelectedClassAbilitiesText;

	TSharedPtr<class SBorder> VanguardCardBorder;
	TSharedPtr<class SBorder> RangerCardBorder;
	TSharedPtr<class SBorder> ArcanistCardBorder;

	void UpdateDetailsView(const FPACharacterClassInfo& Info);
	FReply HandleClassCardClicked(EPACharacterClass InClass);
	FReply HandleEnterWorldClicked();
};
