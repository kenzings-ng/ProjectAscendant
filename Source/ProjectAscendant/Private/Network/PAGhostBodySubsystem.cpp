// Copyright Project Ascendant. All Rights Reserved.

#include "Network/PAGhostBodySubsystem.h"

void UPAGhostBodySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ActiveGhostBodies.Empty();
}

void UPAGhostBodySubsystem::Deinitialize()
{
	ClearAllGhosts();
	Super::Deinitialize();
}

bool UPAGhostBodySubsystem::RegisterDisconnect(const FString& PlayerId, bool bInCombat, float GhostDuration)
{
	// AC-4: Nếu không trong giao tranh, cho phép đăng xuất an toàn ngay lập tức
	if (!bInCombat)
	{
		return false;
	}

	// Đang trong giao tranh: Khởi tạo Xác Ma tồn tại 15.0s trên Dedicated Server
	FPAGhostBodyRecord NewGhost(PlayerId, GhostDuration, true);
	ActiveGhostBodies.Add(PlayerId, NewGhost);

	OnGhostBodySpawned.Broadcast(PlayerId);
	return true;
}

bool UPAGhostBodySubsystem::HandleReconnect(const FString& PlayerId)
{
	if (!ActiveGhostBodies.Contains(PlayerId))
	{
		return false;
	}

	const FPAGhostBodyRecord& Record = ActiveGhostBodies[PlayerId];

	// Nếu Xác Ma đã bị đánh chết trong thời gian rớt mạng, không thể hồi sinh lại
	if (Record.bIsDead)
	{
		return false;
	}

	// Reconnect thành công: Hủy trạng thái Xác Ma và bàn giao lại quyền điều khiển
	ActiveGhostBodies.Remove(PlayerId);
	return true;
}

bool UPAGhostBodySubsystem::TickGhostLifespan(const FString& PlayerId, float DeltaTime)
{
	if (!ActiveGhostBodies.Contains(PlayerId))
	{
		return false;
	}

	FPAGhostBodyRecord& Record = ActiveGhostBodies[PlayerId];
	Record.RemainingGhostTime -= DeltaTime;

	// Hết thời gian 15.0s an toàn
	if (Record.RemainingGhostTime <= 0.0f)
	{
		Record.RemainingGhostTime = 0.0f;
		Record.bIsMarkedForDespawn = true;

		OnGhostBodyDespawned.Broadcast(PlayerId);
		ActiveGhostBodies.Remove(PlayerId);
		return true; // Đã hết hạn và được giải phóng
	}

	return false;
}

bool UPAGhostBodySubsystem::ApplyLethalDamageToGhost(const FString& PlayerId)
{
	if (!ActiveGhostBodies.Contains(PlayerId))
	{
		return false;
	}

	FPAGhostBodyRecord& Record = ActiveGhostBodies[PlayerId];
	Record.bIsDead = true;

	OnGhostBodyKilled.Broadcast(PlayerId);
	ActiveGhostBodies.Remove(PlayerId);
	return true;
}

bool UPAGhostBodySubsystem::IsGhostActive(const FString& PlayerId) const
{
	if (const FPAGhostBodyRecord* Record = ActiveGhostBodies.Find(PlayerId))
	{
		return !Record->bIsDead && !Record->bIsMarkedForDespawn && (Record->RemainingGhostTime > 0.0f);
	}
	return false;
}

float UPAGhostBodySubsystem::GetRemainingGhostTime(const FString& PlayerId) const
{
	if (const FPAGhostBodyRecord* Record = ActiveGhostBodies.Find(PlayerId))
	{
		return Record->RemainingGhostTime;
	}
	return 0.0f;
}

void UPAGhostBodySubsystem::ClearAllGhosts()
{
	ActiveGhostBodies.Empty();
}
