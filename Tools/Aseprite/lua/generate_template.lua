-- ==============================================================================
-- generate_template.lua: Paperdoll Rig Template Generator for Aseprite
-- Project: Project Ascendant (2.5D Isometric Hardcore ARPG MMO on UE 5.8)
-- Conforms to: SPEC-ART-2026-09-23-V2 & character-visual-system.md
-- ==============================================================================

local CANVAS_W = 128
local CANVAS_H = 128
local TOTAL_FRAMES = 16

-- 1. Khởi tạo Sprite mới
local sprite = Sprite(CANVAS_W, CANVAS_H, ColorMode.RGB)
sprite.filename = "Master_Rig_Template.aseprite"

-- 2. Thiết lập Animation Tags & Frame Duration
app.transaction(function()
  -- Tạo đủ 16 frames
  while #sprite.frames < TOTAL_FRAMES do
    sprite:newEmptyFrame()
  end

  -- Frame durations
  for i = 1, 4 do
    sprite.frames[i].duration = 0.150 -- 150ms per frame for Idle (4f loop)
  end
  for i = 5, 10 do
    sprite.frames[i].duration = 0.100 -- 100ms per frame for Walk (6f loop)
  end
  for i = 11, 16 do
    sprite.frames[i].duration = 0.080 -- 80ms per frame for Run (6f loop)
  end

  -- Tags
  local tagIdle = sprite:newTag(1, 4)
  tagIdle.name = "Idle"
  tagIdle.color = Color{ r = 100, g = 200, b = 100 }

  local tagWalk = sprite:newTag(5, 10)
  tagWalk.name = "Walk"
  tagWalk.color = Color{ r = 100, g = 150, b = 255 }

  local tagRun = sprite:newTag(11, 16)
  tagRun.name = "Run"
  tagRun.color = Color{ r = 255, g = 150, b = 100 }
end)

-- 3. Tạo cấu trúc Layers chuẩn Paperdoll 9-Slot
local layer_names = {
  "Layer_Hand_R",
  "Layer_HelmCrest",
  "Layer_Tabard",
  "Layer_UpperBody",
  "Layer_LowerBody",
  "Layer_Hand_L",
  "Guide_Sockets"
}

app.transaction(function()
  -- Xóa layer mặc định
  local defaultLayer = sprite.layers[1]

  local created_layers = {}
  for _, name in ipairs(layer_names) do
    local l = sprite:newLayer()
    l.name = name
    created_layers[name] = l
  end

  if defaultLayer then
    sprite:deleteLayer(defaultLayer)
  end

  -- 4. Vẽ các điểm neo kỹ thuật (Sockets & Alignment Guides) lên Guide_Sockets
  local guideLayer = created_layers["Guide_Sockets"]
  guideLayer.opacity = 180

  local guideColor = Color{ r = 0, g = 255, b = 255, a = 200 }     -- Cyan cho Sockets
  local seamColor = Color{ r = 255, g = 100, b = 100, a = 150 }    -- Red mờ cho Đường nối eo
  local pivotColor = Color{ r = 255, g = 255, b = 0, a = 255 }     -- Yellow cho Điểm tựa chân

  for _, frame in ipairs(sprite.frames) do
    local img = Image(CANVAS_W, CANVAS_H)

    -- A. Đường nối thắt lưng (Waist Seam Y = 80)
    for x = 48, 80 do
      img:drawPixel(x, 80, seamColor)
    end

    -- B. Điểm tiếp đất chân (Foot Pivot Anchor: 64, 114)
    for dx = -2, 2 do
      img:drawPixel(64 + dx, 114, pivotColor)
    end
    for dy = -2, 2 do
      img:drawPixel(64, 114 + dy, pivotColor)
    end

    -- C. Mào nón (Helm Crest Socket: 64, 40)
    for dx = -2, 2 do img:drawPixel(64 + dx, 40, guideColor) end
    for dy = -2, 2 do img:drawPixel(64, 40 + dy, guideColor) end

    -- D. Cờ ngực (Tabard Cutout Socket: 64, 60)
    for dx = -2, 2 do img:drawPixel(64 + dx, 60, guideColor) end
    for dy = -2, 2 do img:drawPixel(64, 60 + dy, guideColor) end

    -- E. Tay phải (HandSocket_R: 96, 76)
    for dx = -2, 2 do img:drawPixel(96 + dx, 76, guideColor) end
    for dy = -2, 2 do img:drawPixel(96, 76 + dy, guideColor) end

    -- F. Tay trái (HandSocket_L: 32, 76)
    for dx = -2, 2 do img:drawPixel(32 + dx, 76, guideColor) end
    for dy = -2, 2 do img:drawPixel(32, 76 + dy, guideColor) end

    sprite:newCel(guideLayer, frame, img)
  end
end)

app.alert("Khởi tạo thành công Paperdoll Template 128x128 với 6 Sockets & 3 Animation Tags!")
