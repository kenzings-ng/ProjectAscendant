// Copyright Project Ascendant. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Account/PAAccountSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * FPAAccountAuthTest
 *
 * Kiểm thử tự động đơn vị cho Hệ Thống Xác Thực & Định Danh Người Chơi (AUTH-SYS Giai Đoạn 1).
 *
 * Tiêu chí kiểm tra:
 *  - 1: Thẩm định cú pháp Email (hợp lệ vs thiếu @ hoặc thiếu tên miền).
 *  - 2: Thẩm định độ dài mật khẩu (tối thiểu 6 ký tự).
 *  - 3: Quy trình đăng ký tài khoản mới và xử lý sai lệch mật khẩu xác nhận / trùng email.
 *  - 4: Chế độ Dev Fast Playtest 1-Click (sinh AccountID, DisplayName và Token tức thì).
 *  - 5: Cơ chế lưu và phục hồi phiên tự động (Remember Me / Auto-Login).
 *  - 6: Xử lý lỗi đăng nhập (sai mật khẩu, tài khoản không tồn tại, đăng xuất an toàn).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAAccountAuthTest,
	"ProjectAscendant.Foundation.Account.AuthAndIdentityPipeline",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAAccountAuthTest::RunTest(const FString& Parameters)
{
	// -------------------------------------------------------------------------
	// Test 1: Thẩm định cú pháp Email (IsValidEmail)
	// -------------------------------------------------------------------------
	{
		// Các email hợp lệ
		TestTrue(TEXT("Email chuẩn phải hợp lệ"), UPAAccountSubsystem::IsValidEmail(TEXT("hero@ascendant.com")));
		TestTrue(TEXT("Email có subdomain phải hợp lệ"), UPAAccountSubsystem::IsValidEmail(TEXT("player_01@sub.domain.org")));
		TestTrue(TEXT("Email có dấu chấm trong tên phải hợp lệ"), UPAAccountSubsystem::IsValidEmail(TEXT("dark.knight@game.net")));

		// Các email không hợp lệ
		TestFalse(TEXT("Email thiếu ký tự @ phải bị từ chối"), UPAAccountSubsystem::IsValidEmail(TEXT("invalid-email.com")));
		TestFalse(TEXT("Email thiếu tên miền domain phải bị từ chối"), UPAAccountSubsystem::IsValidEmail(TEXT("player@")));
		TestFalse(TEXT("Email chứa khoảng trắng phải bị từ chối"), UPAAccountSubsystem::IsValidEmail(TEXT("player name@game.com")));
		TestFalse(TEXT("Email không có dấu chấm domain phải bị từ chối"), UPAAccountSubsystem::IsValidEmail(TEXT("user@localhost")));
		TestFalse(TEXT("Chuỗi rỗng phải bị từ chối"), UPAAccountSubsystem::IsValidEmail(TEXT("")));
	}

	// -------------------------------------------------------------------------
	// Test 2: Thẩm định độ dài mật khẩu (IsValidPassword)
	// -------------------------------------------------------------------------
	{
		FString RequirementError;

		// Mật khẩu >= 6 ký tự
		TestTrue(TEXT("Mật khẩu 6 ký tự phải được chấp thuận"), UPAAccountSubsystem::IsValidPassword(TEXT("123456"), RequirementError));
		TestTrue(TEXT("Mật khẩu dài phức tạp phải được chấp thuận"), UPAAccountSubsystem::IsValidPassword(TEXT("DarkFantasy2026!"), RequirementError));

		// Mật khẩu < 6 ký tự
		TestFalse(TEXT("Mật khẩu 5 ký tự phải bị từ chối"), UPAAccountSubsystem::IsValidPassword(TEXT("12345"), RequirementError));
		TestTrue(TEXT("Thông báo lỗi phải nhắc nhở yêu cầu tối thiểu 6 ký tự"), RequirementError.Contains(TEXT("6")));
	}

	// -------------------------------------------------------------------------
	// Test 3: Quy trình đăng ký tài khoản (RegisterAccount)
	// -------------------------------------------------------------------------
	{
		UPAAccountSubsystem* Subsystem = NewObject<UPAAccountSubsystem>();
		Subsystem->Initialize(*static_cast<FSubsystemCollectionBase*>(nullptr));

		FString ErrorMsg;

		// 3.1: Mật khẩu xác nhận không khớp
		const bool bMismatch = Subsystem->RegisterAccount(TEXT("knight@ascendant.com"), TEXT("pass123"), TEXT("pass456"), ErrorMsg);
		TestFalse(TEXT("Đăng ký với mật khẩu xác nhận không khớp phải thất bại"), bMismatch);
		TestTrue(TEXT("Thông báo lỗi phải chỉ rõ mật khẩu không khớp"), ErrorMsg.Contains(TEXT("không khớp")));

		// 3.2: Đăng ký thành công
		const bool bRegSuccess = Subsystem->RegisterAccount(TEXT("knight@ascendant.com"), TEXT("pass123"), TEXT("pass123"), ErrorMsg);
		TestTrue(TEXT("Đăng ký tài khoản với thông tin chuẩn xác phải thành công"), bRegSuccess);
		TestTrue(TEXT("Thông báo lỗi khi thành công phải rỗng"), ErrorMsg.IsEmpty());

		// 3.3: Đăng ký trùng lặp với cùng email
		const bool bDuplicate = Subsystem->RegisterAccount(TEXT("knight@ascendant.com"), TEXT("anotherpass"), TEXT("anotherpass"), ErrorMsg);
		TestFalse(TEXT("Đăng ký trùng email đã tồn tại phải bị từ chối"), bDuplicate);
		TestTrue(TEXT("Thông báo lỗi phải chỉ rõ tài khoản đã tồn tại"), ErrorMsg.Contains(TEXT("đã tồn tại")));
	}

	// -------------------------------------------------------------------------
	// Test 4: Chế độ Dev Fast Playtest 1-Click (LoginFastPlaytest)
	// -------------------------------------------------------------------------
	{
		UPAAccountSubsystem* Subsystem = NewObject<UPAAccountSubsystem>();
		Subsystem->Initialize(*static_cast<FSubsystemCollectionBase*>(nullptr));

		const bool bFastSuccess = Subsystem->LoginFastPlaytest(TEXT("AlphaTester_99"));
		TestTrue(TEXT("Chế độ Dev Fast Playtest phải đăng nhập thành công tức thì"), bFastSuccess);
		TestTrue(TEXT("Trạng thái subsystem phải là LoggedIn"), Subsystem->IsLoggedIn());

		const FPAAccountProfile& Profile = Subsystem->GetActiveProfile();
		TestEqual(TEXT("Tên hiển thị phải đúng như truyền vào"), Profile.DisplayName, TEXT("AlphaTester_99"));
		TestTrue(TEXT("Profile phải được đánh dấu bIsDevProfile = true"), Profile.bIsDevProfile);
		TestTrue(TEXT("AuthToken phải có tiền tố chuẩn PA-TOKEN-"), Profile.AuthToken.StartsWith(TEXT("PA-TOKEN-")));
		TestTrue(TEXT("AccountID phải có định dạng PA-DEV-"), Profile.AccountID.StartsWith(TEXT("PA-DEV-")));
	}

	// -------------------------------------------------------------------------
	// Test 5: Đăng nhập chuẩn & Cơ chế lưu phiên (Remember Me)
	// -------------------------------------------------------------------------
	{
		UPAAccountSubsystem* Subsystem = NewObject<UPAAccountSubsystem>();
		Subsystem->Initialize(*static_cast<FSubsystemCollectionBase*>(nullptr));

		FString ErrorMsg;

		// 5.1: Đăng nhập bằng tài khoản thử nghiệm mặc định (tester@ascendant.com) kèm Remember Me
		const bool bLoginSuccess = Subsystem->LoginWithCredentials(TEXT("tester@ascendant.com"), TEXT("ascendant123"), true, ErrorMsg);
		TestTrue(TEXT("Đăng nhập bằng tài khoản mặc định kèm Remember Me phải thành công"), bLoginSuccess);
		TestTrue(TEXT("Cờ bHasSavedSession phải được kích hoạt true"), Subsystem->HasSavedSession());

		// 5.2: Đăng xuất an toàn
		Subsystem->Logout();
		TestFalse(TEXT("Sau khi đăng xuất, trạng thái không còn là LoggedIn"), Subsystem->IsLoggedIn());
		TestTrue(TEXT("ActiveProfile phải được dọn rỗng"), Subsystem->GetActiveProfile().AccountID.IsEmpty());
		TestTrue(TEXT("Phiên đăng nhập đã lưu trước đó vẫn còn tồn tại"), Subsystem->HasSavedSession());

		// 5.3: Tự động phục hồi phiên làm việc (AutoLoginWithSavedSession)
		const bool bAutoSuccess = Subsystem->AutoLoginWithSavedSession();
		TestTrue(TEXT("Tự động đăng nhập bằng phiên đã lưu phải thành công"), bAutoSuccess);
		TestTrue(TEXT("Trạng thái người chơi trở lại LoggedIn"), Subsystem->IsLoggedIn());
		TestEqual(TEXT("Tài khoản tự động phục hồi đúng email"), Subsystem->GetActiveProfile().Email, TEXT("tester@ascendant.com"));

		// 5.4: Xóa sạch phiên đã lưu
		Subsystem->ClearSavedSession();
		TestFalse(TEXT("Sau khi ClearSavedSession, HasSavedSession phải trả về false"), Subsystem->HasSavedSession());
	}

	// -------------------------------------------------------------------------
	// Test 6: Xử lý thông báo lỗi đăng nhập (Sai mật khẩu, tài khoản không tồn tại)
	// -------------------------------------------------------------------------
	{
		UPAAccountSubsystem* Subsystem = NewObject<UPAAccountSubsystem>();
		Subsystem->Initialize(*static_cast<FSubsystemCollectionBase*>(nullptr));

		FString ErrorMsg;

		// 6.1: Nhập sai mật khẩu
		const bool bWrongPass = Subsystem->LoginWithCredentials(TEXT("tester@ascendant.com"), TEXT("wrongpassword!"), false, ErrorMsg);
		TestFalse(TEXT("Đăng nhập sai mật khẩu phải thất bại"), bWrongPass);
		TestTrue(TEXT("Thông báo lỗi phải cảnh báo sai mật khẩu"), ErrorMsg.Contains(TEXT("không chính xác")));

		// 6.2: Tài khoản không tồn tại
		const bool bNotFound = Subsystem->LoginWithCredentials(TEXT("nonexistent@nobody.com"), TEXT("password123"), false, ErrorMsg);
		TestFalse(TEXT("Đăng nhập tài khoản chưa đăng ký phải thất bại"), bNotFound);
		TestTrue(TEXT("Thông báo lỗi phải nhắc nhở tài khoản không tồn tại"), ErrorMsg.Contains(TEXT("không tồn tại")));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
