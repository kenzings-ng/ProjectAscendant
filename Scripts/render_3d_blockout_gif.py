import os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from PIL import Image

def draw_box(ax, origin, size, color='#4A5568', edgecolor='#718096', alpha=0.9):
    ox, oy, oz = origin
    dx, dy, dz = size
    
    # 8 vertices of cuboid
    vertices = [
        [ox, oy, oz],
        [ox + dx, oy, oz],
        [ox + dx, oy + dy, oz],
        [ox, oy + dy, oz],
        [ox, oy, oz + dz],
        [ox + dx, oy, oz + dz],
        [ox + dx, oy + dy, oz + dz],
        [ox, oy + dy, oz + dz]
    ]
    
    # 6 faces
    faces = [
        [vertices[0], vertices[1], vertices[2], vertices[3]], # Bottom
        [vertices[4], vertices[5], vertices[6], vertices[7]], # Top
        [vertices[0], vertices[1], vertices[5], vertices[4]], # Front
        [vertices[2], vertices[3], vertices[7], vertices[6]], # Back
        [vertices[0], vertices[3], vertices[7], vertices[4]], # Left
        [vertices[1], vertices[2], vertices[6], vertices[5]]  # Right
    ]
    
    poly = Poly3DCollection(faces, alpha=alpha, facecolor=color, edgecolor=edgecolor, linewidths=0.8)
    ax.add_collection3d(poly)

def render_map_gif():
    output_dir = "ProjectAscendant/Content/art"
    artifact_dir = "/home/kenzings/.gemini/antigravity-cli/brain/83a2074d-86e3-4888-ac19-47b28c1c1c8a"
    os.makedirs(output_dir, exist_ok=True)
    os.makedirs(artifact_dir, exist_ok=True)
    
    gif_path = os.path.join(output_dir, "ruins_blockout_3d.gif")
    artifact_gif_path = os.path.join(artifact_dir, "ruins_blockout_3d.gif")
    
    frames = []
    num_frames = 36 # 36 frames for 360 degree smooth rotation
    
    print("Rendering 3D frames for L_Ruins_Blockout...")
    
    for i in range(num_frames):
        fig = plt.figure(figsize=(8, 6), dpi=100)
        fig.patch.set_facecolor('#0B0E14')
        ax = fig.add_subplot(111, projection='3d')
        ax.set_facecolor('#0B0E14')
        
        # Camera angles: elevation ~ 40-45 deg, rotating azimuth
        angle = (i * 360 / num_frames) + 45.0
        ax.view_init(elev=40, azim=angle)
        
        # 1. Floor grid (60x60m)
        grid_range = np.linspace(-30, 30, 13)
        for gx in grid_range:
            ax.plot([gx, gx], [-30, 30], [0, 0], color='#1E2638', lw=0.7, alpha=0.7)
        for gy in grid_range:
            ax.plot([-30, 30], [gy, gy], [0, 0], color='#1E2638', lw=0.7, alpha=0.7)
            
        # 2. Central Sanctuary Circle (Radius 10m / 1000cm)
        theta = np.linspace(0, 2*np.pi, 60)
        cx = 10 * np.cos(theta)
        cy = 10 * np.sin(theta)
        cz = np.zeros_like(theta) + 0.05
        ax.plot(cx, cy, cz, color='#00E5FF', lw=2.2, alpha=0.9)
        
        # Inner Sanctuary Rune Star
        star_theta = np.linspace(0, 4*np.pi, 6)
        sx = 5 * np.cos(star_theta)
        sy = 5 * np.sin(star_theta)
        sz = np.zeros_like(star_theta) + 0.06
        ax.plot(sx, sy, sz, color='#00B0FF', lw=1.2, alpha=0.75)
        
        # 3. PlayerStart Pedestal (Gold/Cyan)
        draw_box(ax, (-1, -1, 0), (2, 2, 0.4), color='#FFD700', edgecolor='#FFA000', alpha=0.85)
        # Player Marker
        ax.scatter([0], [0], [1.2], color='#00FFA3', s=60, marker='^', edgecolors='#FFFFFF', depthshade=False)
        
        # 4. Stone Pillars (Blockout 2x2x6m)
        pillar_data = [
            ((3, 3, 0), (2, 2, 6), '#3A4454', '#5A6678'),    # NE Occlusion Pillar
            ((-5, -5, 0), (2, 2, 6), '#3A4454', '#5A6678'),  # SW Occlusion Pillar
            ((-8, 6, 0), (2.5, 2.5, 7), '#2D3748', '#4A5568'), # NW Ruin
            ((7, -7, 0), (2.5, 2.5, 7), '#2D3748', '#4A5568'), # SE Ruin
            ((-12, -2, 0), (2, 4, 5), '#2D3748', '#4A5568'),   # West Wall Block
            ((10, 2, 0), (2, 4, 5), '#2D3748', '#4A5568'),    # East Wall Block
            ((0, 12, 0), (5, 2, 4), '#2D3748', '#4A5568'),    # North Wall Block
            ((-2, -14, 0), (5, 2, 4), '#2D3748', '#4A5568'),  # South Wall Block
        ]
        
        for origin, size, col, edge in pillar_data:
            draw_box(ax, origin, size, color=col, edgecolor=edge, alpha=0.92)
            
        # 5. Fixed Isometric Camera Ray indicator (at angle = 45 deg frame)
        cam_x = 0 + 20 * np.cos(np.radians(45))
        cam_y = 0 - 20 * np.sin(np.radians(45))
        cam_z = 18
        ax.plot([cam_x, 0], [cam_y, 0], [cam_z, 1.2], color='#FF5252', linestyle='--', lw=1.2, alpha=0.4)
        
        # Configure axes
        ax.set_xlim(-25, 25)
        ax.set_ylim(-25, 25)
        ax.set_zlim(0, 20)
        
        # Hide standard axis ticks and spines for clean cinematic look
        ax.set_xticks([])
        ax.set_yticks([])
        ax.set_zticks([])
        ax.xaxis.line.set_color('#1E2638')
        ax.yaxis.line.set_color('#1E2638')
        ax.zaxis.line.set_color('#1E2638')
        ax.grid(False)
        
        # Add HUD Title in 3D canvas
        plt.title(f"Project Ascendant — L_Ruins_Blockout (Isometric 45° Pitch)\nSanctuary (10m) | Graybox Pillars | NavMesh", 
                  color='#A0AEC0', fontsize=10, pad=-15)
        
        plt.tight_layout()
        
        # Save to buffer
        fig.canvas.draw()
        image = Image.frombytes('RGBA', fig.canvas.get_width_height(), fig.canvas.buffer_rgba())
        frames.append(image)
        plt.close(fig)
        
    # Save animated GIF
    frames[0].save(
        gif_path,
        save_all=True,
        append_images=frames[1:],
        optimize=True,
        duration=70, # ~14 FPS
        loop=0
    )
    # Also copy to artifacts dir
    frames[0].save(
        artifact_gif_path,
        save_all=True,
        append_images=frames[1:],
        optimize=True,
        duration=70,
        loop=0
    )
    print(f"GIF successfully created at: {gif_path} and {artifact_gif_path}")

if __name__ == "__main__":
    render_map_gif()
