// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Account/PAAccountSubsystem.h"
#include "Input/Reply.h"
#include "PALoginWidget.generated.h"

/**
 * EPALoginViewMode
 *
 * Chế độ hiển thị của màn hình xác thực: Đăng nhập, Đăng ký hoặc Fast Playtest.
 */
UENUM(BlueprintType)
enum class EPALoginViewMode : uint8
{
	Login         UMETA(DisplayName = "Đăng Nhập"),
	Register      UMETA(DisplayName = "Đăng Ký"),
	FastPlaytest  UMETA(DisplayName = "Dev Fast Playtest")
};

/**
 * UPALoginWidget
 *
 * Lớp cơ sở C++ cho giao diện CommonUI Màn hình Đăng Nhập / Đăng Ký
 * phong cách HD-2D Dark Fantasy của Project Ascendant.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTASCENDANT_API UPALoginWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Gửi yêu cầu đăng nhập bằng Email và Mật khẩu */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|UI|Login")
	void RequestLogin(const FString& Email, const FString& Password, bool bRememberMe);

	/** Gửi yêu cầu đăng ký tài khoản mới */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|UI|Login")
	void RequestRegister(const FString& Email, const FString& Password, const FString& ConfirmPassword);

	/** Gửi yêu cầu vào game nhanh bằng chế độ Dev Fast Playtest */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|UI|Login")
	void RequestFastPlaytest(const FString& DevDisplayName);

	/** Chuyển đổi qua lại giữa các chế độ xem */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|UI|Login")
	void SwitchViewMode(EPALoginViewMode NewMode);

	/** Lấy chế độ xem hiện tại */
	UFUNCTION(BlueprintPure, Category = "Ascendant|UI|Login")
	EPALoginViewMode GetCurrentViewMode() const { return CurrentViewMode; }

	/** Cập nhật thông báo trạng thái lên Slate UI */
	void SetStatusText(const FString& Message, bool bIsError);

protected:
	/** Sự kiện Blueprint để hiển thị thông báo trạng thái hoặc lỗi lên UI */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ascendant|UI|Login")
	void OnDisplayStatusMessage(const FString& Message, bool bIsError);

	/** Sự kiện Blueprint khi trạng thái đăng nhập thay đổi */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ascendant|UI|Login")
	void OnLoginStateUpdated(EPAAuthLoginState NewState);

	/** Sự kiện Blueprint khi đăng nhập thành công để kích hoạt chuyển map */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ascendant|UI|Login")
	void OnLoginSucceeded(const FPAAccountProfile& Profile);

	/** Sự kiện Blueprint khi đổi chế độ xem */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ascendant|UI|Login")
	void OnViewModeChanged(EPALoginViewMode NewMode);

	/** Lấy con trỏ tới Account Subsystem từ GameInstance */
	UFUNCTION(BlueprintPure, Category = "Ascendant|UI|Login")
	UPAAccountSubsystem* GetAccountSubsystem() const;

	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	/** Mở màn hình chọn nhân vật (Character Selection) */
	UFUNCTION(BlueprintCallable, Category = "Ascendant|UI|Login")
	class UPACharacterSelectWidget* OpenCharacterSelection();

	/** Lớp Widget Chọn Nhân Vật để khởi tạo sau khi đăng nhập */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|UI|Login")
	TSubclassOf<class UPACharacterSelectWidget> CharacterSelectWidgetClass;

private:
	UFUNCTION()
	void HandleLoginStateChanged(EPAAuthLoginState NewState, const FString& ErrorMessage);

	UFUNCTION()
	void HandleLoginSuccess(const FPAAccountProfile& Profile);

	// Slate UI Components
	TSharedPtr<class SEditableTextBox> LoginEmailBox;
	TSharedPtr<class SEditableTextBox> LoginPasswordBox;
	TSharedPtr<class SCheckBox> RememberMeCheckBox;

	TSharedPtr<class SEditableTextBox> RegisterEmailBox;
	TSharedPtr<class SEditableTextBox> RegisterPasswordBox;
	TSharedPtr<class SEditableTextBox> RegisterConfirmPasswordBox;

	TSharedPtr<class SEditableTextBox> DevDisplayNameBox;
	TSharedPtr<class STextBlock> StatusTextBlock;

	TSharedPtr<class SWidget> LoginTabContainer;
	TSharedPtr<class SWidget> RegisterTabContainer;
	TSharedPtr<class SWidget> FastPlayTabContainer;

	FReply HandleLoginButtonClicked();
	FReply HandleRegisterButtonClicked();
	FReply HandleFastPlayButtonClicked(FString CustomName = TEXT("DevWarrior"));
	FReply HandleSwitchTab(EPALoginViewMode NewMode);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascendant|UI|Login")
	EPALoginViewMode CurrentViewMode = EPALoginViewMode::Login;
};
