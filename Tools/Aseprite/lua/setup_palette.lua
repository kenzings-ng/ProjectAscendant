-- ==============================================================================
-- setup_palette.lua: Official 4-Tone Ramp Palette Setup Script for Aseprite
-- Project: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)
-- Specification: SPEC-ART-2026-09-23-V2
-- ==============================================================================

local sprite = app.activeSprite
if not sprite then
  return app.alert("Vui lòng mở hoặc tạo một Sprite trước khi áp dụng bảng màu.")
end

-- Định nghĩa 32 màu chuẩn theo hệ 4-Tone Ramp Hue-Shifting (Anti-AI Rule)
local palette_colors = {
  -- 0: Trong suốt (Alpha = 0)
  Color{ r = 0, g = 0, b = 0, a = 0 },

  -- 1-2: Viền đen và màu nền tối
  Color{ r = 18, g = 18, b = 20, a = 255 },  -- #121214 (Viền bao ngoài 1px than sẫm)
  Color{ r = 30, g = 36, b = 44, a = 255 },  -- #1E242C (Contour kim loại sẫm)

  -- 3-6: Ramp Thép / Sắt (Steel & Iron)
  Color{ r = 74,  g = 88,  b = 104, a = 255 }, -- #4A5868 (Shadow ngả Navy)
  Color{ r = 148, g = 164, b = 180, a = 255 }, -- #94A4B4 (Midtone)
  Color{ r = 220, g = 228, b = 236, a = 255 }, -- #DCE4EC (Highlight)
  Color{ r = 255, g = 255, b = 255, a = 255 }, -- #FFFFFF (Specular Glint 1px)

  -- 7-10: Ramp Vàng / Đồng Hoàng Kim (Gold & Brass)
  Color{ r = 58,  g = 36,  b = 8,   a = 255 }, -- #3A2408 (Shadow sẫm)
  Color{ r = 140, g = 90,  b = 20,  a = 255 }, -- #8C5A14 (Shadow)
  Color{ r = 224, g = 168, b = 48,  a = 255 }, -- #E0A830 (Midtone)
  Color{ r = 255, g = 244, b = 176, a = 255 }, -- #FFF4B0 (Highlight ngả Vàng chanh)

  -- 11-14: Ramp Da & Gỗ (Leather & Wood)
  Color{ r = 44,  g = 24,  b = 8,   a = 255 }, -- #2C1808 (Shadow sẫm)
  Color{ r = 92,  g = 58,  b = 30,  a = 255 }, -- #5C3A1E (Shadow)
  Color{ r = 154, g = 106, b = 64,  a = 255 }, -- #9A6A40 (Midtone)
  Color{ r = 208, g = 168, b = 120, a = 255 }, -- #D0A878 (Highlight)

  -- 15-18: Ramp Da Người (Skin & Flesh Tone)
  Color{ r = 80,  g = 40,  b = 24,  a = 255 }, -- #502818 (Deep Shadow)
  Color{ r = 176, g = 112, b = 80,  a = 255 }, -- #B07050 (Shadow)
  Color{ r = 232, g = 176, b = 136, a = 255 }, -- #E8B088 (Midtone)
  Color{ r = 255, g = 224, b = 192, a = 255 }, -- #FFE0C0 (Highlight)

  -- 19-22: Ramp Đỏ / Máu / Vải Đỏ (Crimson & Fabric)
  Color{ r = 56,  g = 0,   b = 8,   a = 255 }, -- #380008 (Deep Shadow)
  Color{ r = 128, g = 8,   b = 24,  a = 255 }, -- #800818 (Shadow)
  Color{ r = 208, g = 32,  b = 32,  a = 255 }, -- #D02020 (Midtone)
  Color{ r = 255, g = 112, b = 96,  a = 255 }, -- #FF7060 (Highlight)

  -- 23-26: Ramp Lam / Mana / Áo Choàng Xanh (Blue Fabric)
  Color{ r = 8,   g = 16,  b = 48,  a = 255 }, -- #081030 (Deep Shadow)
  Color{ r = 16,  g = 40,  b = 120, a = 255 }, -- #102878 (Shadow)
  Color{ r = 40,  g = 96,  b = 208, a = 255 }, -- #2860D0 (Midtone)
  Color{ r = 112, g = 176, b = 255, a = 255 }, -- #70B0FF (Highlight)

  -- 27-30: Ramp Tím Hư Không (Void Purple)
  Color{ r = 24,  g = 4,   b = 40,  a = 255 }, -- #180428 (Deep Shadow)
  Color{ r = 64,  g = 16,  b = 104, a = 255 }, -- #401068 (Shadow)
  Color{ r = 128, g = 48,  b = 192, a = 255 }, -- #8030C0 (Midtone)
  Color{ r = 208, g = 136, b = 255, a = 255 }, -- #D088FF (Highlight)

  -- 31: Màu Guide Hỗ Trợ (Cyan Guides)
  Color{ r = 0,   g = 255, b = 255, a = 180 }  -- #00FFFF (Guide Sockets & Alignment)
}

app.transaction(function()
  local pal = Palette(#palette_colors)
  for i, c in ipairs(palette_colors) do
    pal:setColor(i - 1, c)
  end
  sprite:setPalette(pal)
end)

app.alert("Đã nạp thành công Bảng màu 4-Tone Ramp chuẩn cho Project Ascendant (" .. #palette_colors .. " màu)!")
