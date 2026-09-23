// Copyright Project Ascendant. All Rights Reserved.

#include "UI/PALoginWidget.h"
#include "UI/PACharacterSelectWidget.h"
#include "Account/PAAccountSubsystem.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"

void UPALoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UPAAccountSubsystem* Subsystem = GetAccountSubsystem())
	{
		Subsystem->OnLoginStateChanged.AddDynamic(this, &UPALoginWidget::HandleLoginStateChanged);
		Subsystem->OnLoginSuccess.AddDynamic(this, &UPALoginWidget::HandleLoginSuccess);

		// Nếu có phiên đăng nhập đã lưu, tự động thử đăng nhập
		if (Subsystem->HasSavedSession())
		{
			OnDisplayStatusMessage(TEXT("Đang tự động phục hồi phiên làm việc..."), false);
			Subsystem->AutoLoginWithSavedSession();
		}
	}
}

void UPALoginWidget::NativeDestruct()
{
	if (UPAAccountSubsystem* Subsystem = GetAccountSubsystem())
	{
		Subsystem->OnLoginStateChanged.RemoveDynamic(this, &UPALoginWidget::HandleLoginStateChanged);
		Subsystem->OnLoginSuccess.RemoveDynamic(this, &UPALoginWidget::HandleLoginSuccess);
	}

	Super::NativeDestruct();
}

UPAAccountSubsystem* UPALoginWidget::GetAccountSubsystem() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UPAAccountSubsystem>();
	}
	return nullptr;
}

void UPALoginWidget::SetStatusText(const FString& Message, bool bIsError)
{
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(FText::FromString(Message));
		StatusTextBlock->SetColorAndOpacity(FSlateColor(bIsError ? FLinearColor(1.0f, 0.35f, 0.35f, 1.0f) : FLinearColor(0.35f, 0.95f, 0.55f, 1.0f)));
	}
}

void UPALoginWidget::RequestLogin(const FString& Email, const FString& Password, bool bRememberMe)
{
	UPAAccountSubsystem* Subsystem = GetAccountSubsystem();
	if (!Subsystem)
	{
		const FString ErrMsg = TEXT("Lỗi hệ thống: Không thể kết nối với Account Subsystem.");
		SetStatusText(ErrMsg, true);
		OnDisplayStatusMessage(ErrMsg, true);
		return;
	}

	SetStatusText(TEXT("Đang xác thực thông tin đăng nhập..."), false);
	FString ErrorMsg;
	const bool bSuccess = Subsystem->LoginWithCredentials(Email, Password, bRememberMe, ErrorMsg);
	if (!bSuccess && !ErrorMsg.IsEmpty())
	{
		SetStatusText(ErrorMsg, true);
		OnDisplayStatusMessage(ErrorMsg, true);
	}
}

void UPALoginWidget::RequestRegister(const FString& Email, const FString& Password, const FString& ConfirmPassword)
{
	UPAAccountSubsystem* Subsystem = GetAccountSubsystem();
	if (!Subsystem)
	{
		const FString ErrMsg = TEXT("Lỗi hệ thống: Không thể kết nối với Account Subsystem.");
		SetStatusText(ErrMsg, true);
		OnDisplayStatusMessage(ErrMsg, true);
		return;
	}

	SetStatusText(TEXT("Đang gửi yêu cầu khởi tạo tài khoản mới..."), false);
	FString ErrorMsg;
	const bool bSuccess = Subsystem->RegisterAccount(Email, Password, ConfirmPassword, ErrorMsg);
	if (bSuccess)
	{
		const FString SuccessMsg = TEXT("Đăng ký thành công! Hãy đăng nhập để bắt đầu hành trình.");
		SetStatusText(SuccessMsg, false);
		OnDisplayStatusMessage(SuccessMsg, false);
		HandleSwitchTab(EPALoginViewMode::Login);
	}
	else if (!ErrorMsg.IsEmpty())
	{
		SetStatusText(ErrorMsg, true);
		OnDisplayStatusMessage(ErrorMsg, true);
	}
}

void UPALoginWidget::RequestFastPlaytest(const FString& DevDisplayName)
{
	UPAAccountSubsystem* Subsystem = GetAccountSubsystem();
	if (!Subsystem)
	{
		const FString ErrMsg = TEXT("Lỗi hệ thống: Không thể kết nối với Account Subsystem.");
		SetStatusText(ErrMsg, true);
		OnDisplayStatusMessage(ErrMsg, true);
		return;
	}

	SetStatusText(TEXT("Đang kích hoạt phiên Dev Fast Playtest..."), false);
	OnDisplayStatusMessage(TEXT("Đang kích hoạt phiên Dev Fast Playtest..."), false);
	Subsystem->LoginFastPlaytest(DevDisplayName);
}

void UPALoginWidget::SwitchViewMode(EPALoginViewMode NewMode)
{
	CurrentViewMode = NewMode;
	OnViewModeChanged(NewMode);
}

void UPALoginWidget::HandleLoginStateChanged(EPAAuthLoginState NewState, const FString& ErrorMessage)
{
	OnLoginStateUpdated(NewState);

	if (!ErrorMessage.IsEmpty())
	{
		SetStatusText(ErrorMessage, true);
		OnDisplayStatusMessage(ErrorMessage, true);
	}
}

void UPALoginWidget::HandleLoginSuccess(const FPAAccountProfile& Profile)
{
	const FString SuccessMsg = FString::Printf(TEXT("Chào mừng trở lại, %s!"), *Profile.DisplayName);
	SetStatusText(SuccessMsg, false);
	OnDisplayStatusMessage(SuccessMsg, false);
	OnLoginSucceeded(Profile);

	OpenCharacterSelection();
}

UPACharacterSelectWidget* UPALoginWidget::OpenCharacterSelection()
{
	TSubclassOf<UPACharacterSelectWidget> TargetClass = CharacterSelectWidgetClass;
	if (!TargetClass)
	{
		TargetClass = UPACharacterSelectWidget::StaticClass();
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		UPACharacterSelectWidget* CharSelectWidget = CreateWidget<UPACharacterSelectWidget>(PC, TargetClass);
		if (CharSelectWidget)
		{
			RemoveFromParent();
			CharSelectWidget->AddToViewport(100);

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(CharSelectWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;

			return CharSelectWidget;
		}
	}

	return nullptr;
}

FReply UPALoginWidget::HandleLoginButtonClicked()
{
	const FString Email = LoginEmailBox.IsValid() ? LoginEmailBox->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Password = LoginPasswordBox.IsValid() ? LoginPasswordBox->GetText().ToString() : FString();
	const bool bRemember = RememberMeCheckBox.IsValid() && RememberMeCheckBox->IsChecked();

	RequestLogin(Email, Password, bRemember);
	return FReply::Handled();
}

FReply UPALoginWidget::HandleRegisterButtonClicked()
{
	const FString Email = RegisterEmailBox.IsValid() ? RegisterEmailBox->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Password = RegisterPasswordBox.IsValid() ? RegisterPasswordBox->GetText().ToString() : FString();
	const FString Confirm = RegisterConfirmPasswordBox.IsValid() ? RegisterConfirmPasswordBox->GetText().ToString() : FString();

	RequestRegister(Email, Password, Confirm);
	return FReply::Handled();
}

FReply UPALoginWidget::HandleFastPlayButtonClicked(FString CustomName)
{
	FString Name = DevDisplayNameBox.IsValid() ? DevDisplayNameBox->GetText().ToString().TrimStartAndEnd() : FString();
	if (Name.IsEmpty())
	{
		Name = CustomName.IsEmpty() ? TEXT("DevWarrior") : CustomName;
	}

	RequestFastPlaytest(Name);
	return FReply::Handled();
}

FReply UPALoginWidget::HandleSwitchTab(EPALoginViewMode NewMode)
{
	SwitchViewMode(NewMode);
	if (LoginTabContainer.IsValid())
	{
		LoginTabContainer->SetVisibility(NewMode == EPALoginViewMode::Login ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (RegisterTabContainer.IsValid())
	{
		RegisterTabContainer->SetVisibility(NewMode == EPALoginViewMode::Register ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (FastPlayTabContainer.IsValid())
	{
		FastPlayTabContainer->SetVisibility(NewMode == EPALoginViewMode::FastPlaytest ? EVisibility::Visible : EVisibility::Collapsed);
	}
	return FReply::Handled();
}

TSharedRef<SWidget> UPALoginWidget::RebuildWidget()
{
	const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 24);
	const FSlateFontInfo SubtitleFont = FCoreStyle::GetDefaultFontStyle("Italic", 11);
	const FSlateFontInfo LabelFont = FCoreStyle::GetDefaultFontStyle("Regular", 11);
	const FSlateFontInfo ButtonFont = FCoreStyle::GetDefaultFontStyle("Bold", 12);
	const FSlateFontInfo StatusFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);

	const FLinearColor GoldColor(0.85f, 0.72f, 0.38f, 1.0f);
	const FLinearColor SubtitleColor(0.65f, 0.65f, 0.70f, 1.0f);
	const FLinearColor DarkBgColor(0.05f, 0.06f, 0.08f, 0.94f);
	const FLinearColor CardBgColor(0.09f, 0.10f, 0.13f, 0.90f);

	// Login Tab Container
	LoginTabContainer = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 6.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("ProjectAscendant", "LoginEmailLabel", "Địa Chỉ Email:"))
			.Font(LabelFont)
			.ColorAndOpacity(FSlateColor(SubtitleColor))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SAssignNew(LoginEmailBox, SEditableTextBox)
			.Text(FText::FromString(TEXT("player@ascendant.realm")))
			.HintText(NSLOCTEXT("ProjectAscendant", "EmailHint", "nhap.email@domain.com"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("ProjectAscendant", "LoginPasswordLabel", "Mật Khẩu:"))
			.Font(LabelFont)
			.ColorAndOpacity(FSlateColor(SubtitleColor))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SAssignNew(LoginPasswordBox, SEditableTextBox)
			.Text(FText::FromString(TEXT("Ascendant2026!")))
			.IsPassword(true)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f, 0.0f, 12.0f)
		[
			SAssignNew(RememberMeCheckBox, SCheckBox)
			.IsChecked(ECheckBoxState::Checked)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("ProjectAscendant", "RememberMeLabel", " Ghi nhớ phiên đăng nhập (Auto-Login)"))
				.Font(LabelFont)
				.ColorAndOpacity(FSlateColor(SubtitleColor))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f, 0.0f, 4.0f)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.ButtonColorAndOpacity(FLinearColor(0.20f, 0.16f, 0.10f, 1.0f))
			.OnClicked_UObject(this, &UPALoginWidget::HandleLoginButtonClicked)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("ProjectAscendant", "LoginBtnText", "⚔  ĐĂNG NHẬP VÀO THẾ GIỚI  ⚔"))
				.Font(ButtonFont)
				.ColorAndOpacity(FSlateColor(GoldColor))
				.Margin(FMargin(16.0f, 8.0f))
			]
		];

	// Register Tab Container
	RegisterTabContainer = SNew(SVerticalBox)
		.Visibility(EVisibility::Collapsed)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 6.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("ProjectAscendant", "RegEmailLabel", "Địa Chỉ Email Mới:"))
			.Font(LabelFont)
			.ColorAndOpacity(FSlateColor(SubtitleColor))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SAssignNew(RegisterEmailBox, SEditableTextBox)
			.HintText(NSLOCTEXT("ProjectAscendant", "RegEmailHint", "nhap.email.moi@domain.com"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("ProjectAscendant", "RegPasswordLabel", "Mật Khẩu (Ít nhất 8 ký tự):"))
			.Font(LabelFont)
			.ColorAndOpacity(FSlateColor(SubtitleColor))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SAssignNew(RegisterPasswordBox, SEditableTextBox)
			.IsPassword(true)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("ProjectAscendant", "RegConfirmLabel", "Xác Nhận Mật Khẩu:"))
			.Font(LabelFont)
			.ColorAndOpacity(FSlateColor(SubtitleColor))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 12.0f)
		[
			SAssignNew(RegisterConfirmPasswordBox, SEditableTextBox)
			.IsPassword(true)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f, 0.0f, 4.0f)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.ButtonColorAndOpacity(FLinearColor(0.12f, 0.18f, 0.22f, 1.0f))
			.OnClicked_UObject(this, &UPALoginWidget::HandleRegisterButtonClicked)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("ProjectAscendant", "RegBtnText", "✦  TẠO TÀI KHOẢN MỚI  ✦"))
				.Font(ButtonFont)
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.85f, 1.0f)))
				.Margin(FMargin(16.0f, 8.0f))
			]
		];

	// Dev Fast Playtest Tab Container
	FastPlayTabContainer = SNew(SVerticalBox)
		.Visibility(EVisibility::Collapsed)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 6.0f, 0.0f, 8.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("ProjectAscendant", "FastPlayDesc", "Chế độ kiểm thử nhanh (1-Click) dành cho Developer & QA. Khởi tạo tài khoản giả lập tức thì để kiểm thử hệ thống chọn nhân vật và chiến đấu."))
			.Font(LabelFont)
			.ColorAndOpacity(FSlateColor(SubtitleColor))
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 2.0f, 0.0f, 2.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("ProjectAscendant", "DevNameLabel", "Tên Nhân Vật Dev:"))
			.Font(LabelFont)
			.ColorAndOpacity(FSlateColor(SubtitleColor))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 12.0f)
		[
			SAssignNew(DevDisplayNameBox, SEditableTextBox)
			.Text(FText::FromString(TEXT("DevHero_01")))
			.HintText(NSLOCTEXT("ProjectAscendant", "DevNameHint", "DevHero_01"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f, 0.0f, 4.0f)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.ButtonColorAndOpacity(FLinearColor(0.18f, 0.14f, 0.22f, 1.0f))
			.OnClicked(FOnClicked::CreateUObject(this, &UPALoginWidget::HandleFastPlayButtonClicked, FString(TEXT("DevWarrior"))))
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("ProjectAscendant", "FastPlayBtnText", "⚡  1-CLICK VÀO CHỌN NHÂN VẬT  ⚡"))
				.Font(ButtonFont)
				.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.7f, 1.0f)))
				.Margin(FMargin(16.0f, 8.0f))
			]
		];

	// Root Widget
	return SNew(SBorder)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
		[
			SNew(SBox)
			.WidthOverride(540.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(DarkBgColor)
				.Padding(FMargin(24.0f, 20.0f))
				[
					SNew(SVerticalBox)
					// Header: Game Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 2.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("ProjectAscendant", "GameTitle", "PROJECT ASCENDANT"))
						.Font(TitleFont)
						.ColorAndOpacity(FSlateColor(GoldColor))
					]
					// Subtitle
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 14.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("ProjectAscendant", "GameSubtitle", "2.5D Isometric HD-2D Dark Fantasy Action RPG"))
						.Font(SubtitleFont)
						.ColorAndOpacity(FSlateColor(SubtitleColor))
					]
					// Tab Switcher Header (3 Tabs)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 12.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(2.0f, 0.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.18f, 1.0f))
							.OnClicked(FOnClicked::CreateUObject(this, &UPALoginWidget::HandleSwitchTab, EPALoginViewMode::Login))
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("ProjectAscendant", "TabLogin", "Đăng Nhập"))
								.Font(LabelFont)
								.ColorAndOpacity(FSlateColor(GoldColor))
								.Margin(FMargin(4.0f, 6.0f))
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(2.0f, 0.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.18f, 1.0f))
							.OnClicked(FOnClicked::CreateUObject(this, &UPALoginWidget::HandleSwitchTab, EPALoginViewMode::Register))
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("ProjectAscendant", "TabRegister", "Đăng Ký"))
								.Font(LabelFont)
								.ColorAndOpacity(FSlateColor(SubtitleColor))
								.Margin(FMargin(4.0f, 6.0f))
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(2.0f, 0.0f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.18f, 1.0f))
							.OnClicked(FOnClicked::CreateUObject(this, &UPALoginWidget::HandleSwitchTab, EPALoginViewMode::FastPlaytest))
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("ProjectAscendant", "TabFastPlay", "Fast Play"))
								.Font(LabelFont)
								.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.7f, 1.0f)))
								.Margin(FMargin(4.0f, 6.0f))
							]
						]
					]
					// Form Card Container
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 12.0f)
					[
						SNew(SBorder)
						.BorderBackgroundColor(CardBgColor)
						.Padding(FMargin(16.0f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()[ LoginTabContainer.ToSharedRef() ]
							+ SVerticalBox::Slot().AutoHeight()[ RegisterTabContainer.ToSharedRef() ]
							+ SVerticalBox::Slot().AutoHeight()[ FastPlayTabContainer.ToSharedRef() ]
						]
					]
					// Status Text Block
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 4.0f)
					[
						SAssignNew(StatusTextBlock, STextBlock)
						.Text(NSLOCTEXT("ProjectAscendant", "DefaultStatus", "Vui lòng đăng nhập hoặc chọn Fast Playtest để bắt đầu."))
						.Font(StatusFont)
						.ColorAndOpacity(FSlateColor(SubtitleColor))
						.AutoWrapText(true)
					]
				]
			]
		];
}
