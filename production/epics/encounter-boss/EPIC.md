# Epic: Encounter & Boss Mechanics Layer

> **Layer**: Encounter / AI  
> **GDD**: [`design/gdd/boss-ai.md`](../../design/gdd/boss-ai.md), [`design/gdd/stagger-system.md`](../../design/gdd/stagger-system.md), [`design/gdd/dash-evasion.md`](../../design/gdd/dash-evasion.md)  
> **Architecture Module**: Combat & AI Architecture ([`docs/architecture/architecture.md`](../../docs/architecture/architecture.md))  
> **Status**: Ready  
> **Stories**: 4 Stories Planned  

## Overview

Hiện thực hóa toàn bộ vòng lặp giao chiến với Trùm (**Boss Encounter Loop**) và cơ chế khắc chế đòn đánh cấp độ cao cho *Project Ascendant*:
1. **Boss AI & Telegraph System (`boss-001`)**: Triển khai Behavior Tree trùm **Ironclad Warlord** với 4 pha ra chiêu chuẩn hóa (Telegraph 0.5-0.8s → Flash Cue 0.10s → Hitbox 0.15-0.25s → Recovery 0.6-1.0s), 5 kỹ năng chiến đấu, EQS chọn hành động theo góc và cự ly, trạng thái Wall Crash Stun 1.8s.
2. **Posture Stagger & Execution (`stgr-001`)**: Xử lý logic phá vỡ thế trận (Posture Break tại 100%), trùm quỳ gối 3.0s (`State.Staggered`), Motion Warping kéo người chơi vào điểm yếu cự ly <250cm, tung đòn kết liễu gây **25% Max HP True Damage**, cấp I-Frame 1.2s và hồi 50% Posture nếu bỏ lỡ.
3. **Anatomical Part Breaking (`stgr-002`)**: Theo dõi sát thương độc lập cho các bộ phận (Sừng 20% HP, Đuôi 15% HP, Giáp ngực 25% HP), loại bỏ vĩnh viễn các kỹ năng tương ứng khỏi Behavior Tree khi bộ phận gãy, kích hoạt điểm yếu ngực (+50% sát thương) và sinh vật phẩm rơi đặc thù.
4. **Dash I-Frame & Perfect Dodge (`dash-001`)**: Hoàn thiện kỹ năng Dash 0.45s với 0.28s bất tử (`State.Invulnerable`), cửa sổ phản xạ hoàng kim (Sweet Spot 0.05s–0.15s) kích hoạt Perfect Dodge (+15 thể lực hoàn lại, 0.08s hitstop toàn cục, banner "PERFECT!"), cùng cơ chế chống trượt mép vực.

## Stories

| ID | Title | Type | Status | Governing ADR |
|---|---|---|---|---|
| 001 | [`Prototype Boss AI & 4-Phase Telegraph System`](story-001-boss-ai-telegraphs.md) | AI / Combat | ✅ Done | ADR-0001, ADR-0002 |
| 002 | [`Posture Stagger, Kneel Window & True Damage Execution`](story-002-stagger-execution.md) | Combat | ✅ Done | ADR-0001, ADR-0002 |
| 003 | [`Anatomical Part Breaking & Skill Disabling Matrix`](story-003-part-breaking-matrix.md) | Combat / AI | Ready | ADR-0001, ADR-0002 |
| 004 | [`Dash I-Frame, Perfect Dodge Sweet Spot & Hitstop`](story-004-dash-iframe-perfect-dodge.md) | Combat / Movement | Ready | ADR-0001, ADR-0002 |

## Definition of Done
- [ ] 100% các Stories đạt chuẩn Acceptance Criteria.
- [ ] Logic trùm, vỡ thế và né đòn được mô hình hóa bằng dữ liệu thuần túy (Pure Data Model), hỗ trợ headless automation tests.
- [ ] Tích hợp mượt mà với UI lớp Presentation đã xây dựng ở Sprint 4 (Boss Health/Posture HUD, Reticle, Perfect Dodge Flash).
- [ ] Tất cả unit/automation tests chạy Pass 100%.
