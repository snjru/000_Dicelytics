import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.widgets import TextBox, Button
from pathlib import Path

def get_project_root() -> Path:
    """Returns the absolute path to the project root directory (dicelytics/)."""
    return Path(__file__).resolve().parent.parent

class DicePlotterGUI:
    def __init__(self, csv_relative_path: str = "data/synthetic/dice_pips_dataset.csv"):
        project_root = get_project_root()
        self.csv_path = project_root / csv_relative_path

        if not self.csv_path.exists():
            print(f"[ERROR] CSV file not found: {self.csv_path}")
            print("First, run: py scripts/pip_generator.py")
            return

        # Load all CSV records
        self.df = pd.read_csv(self.csv_path)
        self.max_sample_id = self.df['sample_id'].max()
        self.current_sample_id = 1
        self.is_playing = False
        self.interval_sec = 1.0  # Default slideshow interval in seconds

        # Set up plot figure and layout
        self.fig, self.ax = plt.subplots(figsize=(7, 8.5))
        plt.subplots_adjust(bottom=0.20)
        self.ax.set_aspect('equal')

        # Define widget areas
        # Row 1: Sample ID input, Interval (s) input
        ax_box_id  = plt.axes([0.18, 0.09, 0.15, 0.04])
        ax_box_sec = plt.axes([0.55, 0.09, 0.12, 0.04])
        # Row 2: Prev, Play/Pause, Next buttons
        ax_btn_prev = plt.axes([0.18, 0.03, 0.18, 0.04])
        ax_btn_play = plt.axes([0.41, 0.03, 0.18, 0.04])
        ax_btn_next = plt.axes([0.64, 0.03, 0.18, 0.04])

        # Create control UI widgets (English labels to prevent CJK font garbling)
        self.text_box_id  = TextBox(ax_box_id, 'Sample ID: ', initial="1")
        self.text_box_sec = TextBox(ax_box_sec, 'Interval (s): ', initial="1.0")
        
        self.btn_prev = Button(ax_btn_prev, 'Prev')
        self.btn_play = Button(ax_btn_play, 'Play')
        self.btn_next = Button(ax_btn_next, 'Next')

        # Connect event callbacks
        self.text_box_id.on_submit(self.on_id_submit)
        self.text_box_sec.on_submit(self.on_sec_submit)
        self.btn_prev.on_clicked(self.prev_sample)
        self.btn_play.on_clicked(self.toggle_play)
        self.btn_next.on_clicked(self.next_sample)

        # Initialize automatic slideshow timer
        self.timer = self.fig.canvas.new_timer(interval=int(self.interval_sec * 1000))
        self.timer.add_callback(self.auto_advance)

        # Initial plot rendering
        self.draw_sample(self.current_sample_id)
        plt.show()

    def draw_sample(self, sample_id: int):
        """Renders the dice and pips layout for a specified sample_id."""
        self.ax.clear()
        self.current_sample_id = sample_id
        
        # Synchronize text box display
        self.text_box_id.set_val(str(sample_id))

        sample_data = self.df[self.df['sample_id'] == sample_id]

        if sample_data.empty:
            self.ax.text(0, 0, f"Sample ID: {sample_id}\nNot Found (Max: {self.max_sample_id})",
                         ha='center', va='center', color='red', fontsize=14)
            self.ax.set_xlim(-25, 25)
            self.ax.set_ylim(-25, 25)
            self.fig.canvas.draw_idle()
            return

        # 1. Draw 40mm boundary circle (radius = 20mm)
        boundary_circle = patches.Circle((0, 0), radius=20.0, fill=False, edgecolor='gray',
                                         linestyle='--', linewidth=1.5, label='Boundary (Ø40mm)')
        self.ax.add_patch(boundary_circle)

        # 2. Draw outer boundary and pips for each dice
        colors = ['#1f77b4', '#ff7f0e', '#2ca02c']
        dice_ids = sample_data['dice_id'].unique()

        for dice_id in dice_ids:
            dice_pips = sample_data[sample_data['dice_id'] == dice_id]
            color = colors[(int(dice_id) - 1) % len(colors)]
            
            pip_xs = dice_pips['pip_x_mm'].values
            pip_ys = dice_pips['pip_y_mm'].values
            cx, cy = pip_xs.mean(), pip_ys.mean()

            # Retrieve rotation angle if dice_deg column exists
            dice_deg = dice_pips['dice_deg'].iloc[0] if 'dice_deg' in dice_pips.columns else 0.0

            # Draw square dice frame (8mm x 8mm)
            dice_size = 8.0
            # [FIXED] Correct calculation for Rectangle bottom-left corner to align center
            square = patches.Rectangle(
                (cx - dice_size / 2.0, cy - dice_size / 2.0),
                dice_size, dice_size,
                angle=dice_deg,
                rotation_point='center',  # Requires Matplotlib >= 3.6
                fill=True, facecolor=color, alpha=0.15, edgecolor=color, linewidth=2,
                label=f'Dice {dice_id} (Val: {dice_pips["pip_value"].iloc[0]})'
            )
            self.ax.add_patch(square)

            # Draw center mark and pips
            self.ax.plot(cx, cy, marker='+', color=color, markersize=8)
            self.ax.scatter(pip_xs, pip_ys, color=color, s=50, zorder=5)

        # Configure axis boundaries and labels
        self.ax.set_xlim(-25, 25)
        self.ax.set_ylim(-25, 25)
        self.ax.set_xlabel("X (mm)")
        self.ax.set_ylabel("Y (mm)")
        status_text = " [PLAYING...]" if self.is_playing else ""
        self.ax.set_title(f"Dice Pips Synthetic Data (Sample ID: {sample_id} / {self.max_sample_id}){status_text}")
        self.ax.grid(True, linestyle=':', alpha=0.6)

        # [FIXED] Remove duplicate entries from the legend
        handles, labels = self.ax.get_legend_handles_labels()
        by_label = dict(zip(labels, handles))
        self.ax.legend(by_label.values(), by_label.keys(), loc='upper right', fontsize='small')

        self.fig.canvas.draw_idle()

    def auto_advance(self):
        """Timer callback to step to the next sample automatically."""
        next_id = self.current_sample_id + 1
        if next_id > self.max_sample_id:
            next_id = 1  # Loop back to start
        self.draw_sample(next_id)

    def toggle_play(self, event=None):
        """Toggles slideshow playback state."""
        if self.is_playing:
            self.timer.stop()
            self.is_playing = False
            self.btn_play.label.set_text('Play')
        else:
            self.timer.start()
            self.is_playing = True
            self.btn_play.label.set_text('Pause')
        self.draw_sample(self.current_sample_id)

    def next_sample(self, event=None):
        """Steps forward to the next sample."""
        next_id = self.current_sample_id + 1 if self.current_sample_id < self.max_sample_id else 1
        self.draw_sample(next_id)

    def prev_sample(self, event=None):
        """Steps back to the previous sample."""
        prev_id = self.current_sample_id - 1 if self.current_sample_id > 1 else self.max_sample_id
        self.draw_sample(prev_id)

    def on_id_submit(self, text: str):
        """Handles manual Sample ID text box submission."""
        try:
            sample_id = int(text)
            self.draw_sample(sample_id)
        except ValueError:
            print("[WARN] Please enter a valid integer for Sample ID.")

    def on_sec_submit(self, text: str):
        """Handles slideshow interval update submission."""
        try:
            sec = float(text)
            if sec < 0.1:
                sec = 0.1  # Prevent excessively high refresh rate
            self.interval_sec = sec
            self.timer.interval = int(self.interval_sec * 1000)
            print(f"[INFO] Updated interval to {self.interval_sec} second(s).")
        except ValueError:
            print("[WARN] Please enter a valid numeric value for interval.")

if __name__ == "__main__":
    DicePlotterGUI()