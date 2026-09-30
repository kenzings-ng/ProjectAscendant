#!/usr/bin/env bash
# ==============================================================================
# Project Ascendant - Game & Editor Launcher
# 2.5D Isometric HD-2D Dark Fantasy Action RPG / MMO
# ==============================================================================

set -e

PROJECT_DIR="/mnt/Data/Projects/project-games/ProjectAscendant"
UPROJECT="${PROJECT_DIR}/ProjectAscendant.uproject"
ENGINE_BIN="/mnt/Data/Engine/Binaries/Linux/UnrealEditor"

MODE="${1:-game}"
RES_X="${2:-1280}"
RES_Y="${3:-720}"

echo "============================================================"
echo " Project Ascendant Launcher"
echo " Engine: Unreal Engine 5.8 (Linux x86_64)"
echo " Mode  : ${MODE} (${RES_X}x${RES_Y})"
echo "============================================================"

# Cấu hình GPU Offload tối ưu cho kiến trúc Hybrid Linux (Intel Display + NVIDIA P104-100 Headless 3D)
export __NV_PRIME_RENDER_OFFLOAD=1
export __GLX_VENDOR_LIBRARY_NAME=nvidia
export __VK_LAYER_NV_optimus=NVIDIA_only
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/nvidia_icd.json

# Khắc phục lỗi dma-buf swapchain trắng trên Wayland với secondary headless GPU: chạy qua Xwayland / X11
export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-x11}"

if [ ! -f "${ENGINE_BIN}" ]; then
    echo "[ERROR] UnrealEditor binary not found at: ${ENGINE_BIN}"
    exit 1
fi

if [ ! -f "${UPROJECT}" ]; then
    echo "[ERROR] .uproject file not found at: ${UPROJECT}"
    exit 1
fi

case "${MODE}" in
    "game"|"play"|"client")
        WINDOW_FLAG="-Windowed"
        if [ "${RES_X}" = "1920" ] && [ "${RES_Y}" = "1080" ]; then
            # Tránh lỗi KDE window manager ép resize cửa sổ 1920x1080 làm treo Vulkan swapchain
            WINDOW_FLAG="-Fullscreen"
            echo "[INFO] Running in Fullscreen mode for 1920x1080 to prevent swapchain resize deadlock."
        fi
        echo "[INFO] Launching Project Ascendant in Standalone Client Mode (${RES_X}x${RES_Y} ${WINDOW_FLAG})..."
        echo "[INFO] Game log is saved to Saved/Logs/ProjectAscendant.log"
        exec "${ENGINE_BIN}" "${UPROJECT}" -game -ResX="${RES_X}" -ResY="${RES_Y}" "${WINDOW_FLAG}"
        ;;
    "game-log")
        echo "[INFO] Launching Project Ascendant with live terminal log..."
        exec "${ENGINE_BIN}" "${UPROJECT}" -game -ResX="${RES_X}" -ResY="${RES_Y}" -Windowed -log
        ;;
    "editor"|"gui")
        echo "[INFO] Launching Unreal Editor GUI..."
        exec "${ENGINE_BIN}" "${UPROJECT}"
        ;;
    "server")
        echo "[INFO] Launching Dedicated Server..."
        exec "${ENGINE_BIN}" "${UPROJECT}" -server -log -PORT=7777
        ;;
    *)
        echo "Usage: ./run_game.sh [game|game-log|editor|server] [ResX] [ResY]"
        echo "Examples:"
        echo "  ./run_game.sh                  # Chạy game Standalone Cửa sổ 1280x720 (Khuyến nghị)"
        echo "  ./run_game.sh game 1920 1080   # Chạy game Toàn màn hình 1920x1080"
        echo "  ./run_game.sh game-log         # Chạy kèm cửa sổ console log debug"
        echo "  ./run_game.sh editor           # Mở giao diện Unreal Editor"
        echo "  ./run_game.sh server           # Khởi chạy Dedicated Server cục bộ"
        exit 1
        ;;
esac
