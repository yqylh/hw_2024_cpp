import tkinter as tk
import numpy as np
import argparse

result = np.zeros((200, 200))

help = "\
定义: 0 : 障碍物 \n \
      1 : 陆地 (通行区域) \n \
      2 : 海洋 \n \
      3 : 泊位 \n \
      4 : 机器人出生点 \n \
    按e切换单选、框选\n \
    按s保存地图\n \
"

BLOCK_COLOR = (156, 140, 128) 
TEXT_COLOR =  (255, 255, 255)
EARTH_COLOR = (201, 223, 153)
OCEAN_COLOR = (206, 222, 254)
ROBOT_COLOR = (0, 0, 255)
SHIP_COLOR =  (251, 241, 51)
PORT_COLOR =  (190, 193, 198)
ITEM_COLOR =  (249, 139, 228)

color_map = [BLOCK_COLOR, EARTH_COLOR, OCEAN_COLOR, PORT_COLOR, ROBOT_COLOR, SHIP_COLOR]
map_name = None


now_type = 0

class MapEditor:
    def __init__(self, master,map_height=200,map_width=200,cell_size=20):
        self.master = master
        self.map_height = map_height
        self.map_width = map_width
        self.cell_size = cell_size  # 每个格子的大小（像素），增加格子大小以便于查看和操作
        self.canvas = tk.Canvas(master, width=map_width * self.cell_size, height=map_height * self.cell_size)
        self.canvas.grid(row=0, column=0)
        self.text_display = tk.Text(master, height=25, width=30)  # Text widget for displaying keyboard input
        self.text_display.grid(row=0, column=1, padx=10, pady=10)
        # self.canvas.pack()
        self.draw_grid()
        self.choose_type = 0 # 0 for 单选，1 for 框选
        self.canvas.bind("<Button-1>", self.cell_choose_start)
        self.canvas.bind("<ButtonRelease-1>", self.cell_choose_end)
        self.canvas.bind("<B1-Motion>", self.cell_choose)
        self.canvas.bind("<B3-Motion>", self.cell_remove)
        self.master.bind("<Key>", self.key_input)
        self.text_display.insert(tk.END, help)
        self.canvas.focus_set()

        self.now_type = 0
        self.now_color = color_map[self.now_type]

    def rgbtofill(self,rgb):
        return '#%02x%02x%02x' % rgb
    
    def key_input(self, event):
        # Update text_display with the key pressed
        # self.text_display.insert(tk.END, event.char)
        self.text_display.delete(1.0, tk.END)
        self.text_display.insert(tk.END, help)
        self.canvas.focus_set()  # Refocus to canvas after key input
        if event.char == 's':
            save_map()
        if event.char == 'e':
            self.choose_type = 1 if self.choose_type == 0 else 0
            self.text_display.insert(tk.END, "Choose type: " + str(self.choose_type) + "\n")
        elif event.char == 'q':
            exit()
        elif event.char in ['0', '1', '2', '3', '4']:
            self.now_type = int(event.char)
            self.now_color = color_map[self.now_type]
            self.text_display.insert(tk.END, "Input type: " + str(self.now_type) + "\n")
        else:
            self.text_display.insert(tk.END, "Invalid input: " + event.char + "\n")

    def draw_grid(self):
        for i in range(self.map_width * self.cell_size):
            for j in range(self.map_height):
                self.canvas.create_rectangle(i*self.cell_size, j*self.cell_size, (i+1)*self.cell_size, (j+1)*self.cell_size, fill=self.rgbtofill(BLOCK_COLOR), outline="gray")

    def cell_choose(self, event):
        if self.choose_type == 0:
            grid_x, grid_y = event.x // self.cell_size, event.y // self.cell_size
            result[grid_y][grid_x] = self.now_type
            self.canvas.create_rectangle(grid_x*self.cell_size, grid_y*self.cell_size, (grid_x+1)*self.cell_size, (grid_y+1)*self.cell_size, fill=self.rgbtofill(self.now_color), outline="gray")

    def cell_remove(self, event):
        if self.choose_type == 0:
            grid_x, grid_y = event.x // self.cell_size, event.y // self.cell_size
            result[grid_y][grid_x] = 0
            self.canvas.create_rectangle(grid_x*self.cell_size, grid_y*self.cell_size, (grid_x+1)*self.cell_size, (grid_y+1)*self.cell_size, fill=self.rgbtofill(BLOCK_COLOR), outline="gray")

    def cell_choose_start(self, event):
        if self.choose_type == 1:
            self.start_x, self.start_y = event.x // self.cell_size, event.y // self.cell_size
        else:
            self.cell_choose(event)
    
    def cell_choose_end(self, event):
        if self.choose_type == 1:
            self.end_x, self.end_y = event.x // self.cell_size, event.y // self.cell_size
            for i in range(min(self.start_x, self.end_x), max(self.start_x, self.end_x)):
                for j in range(min(self.start_y, self.end_y), max(self.start_y, self.end_y)):
                    result[j][i] = self.now_type
                    self.canvas.create_rectangle(i*self.cell_size, j*self.cell_size, (i+1)*self.cell_size, (j+1)*self.cell_size, fill=self.rgbtofill(self.now_color), outline="gray")
            self.choose_type = 0

def save_map():
    print("Saving map as " + map_name)
    with open(map_name, 'w') as map_file:
        for line in result:
            for grid_content in line:
                if grid_content == 0:
                    map_file.write('#')
                if grid_content == 1:
                    map_file.write('.')
                if grid_content == 2:
                    map_file.write('*')
                if grid_content == 3:
                    map_file.write('B')
                if grid_content == 4:
                    map_file.write('A')
            map_file.write('\n')
    print("Map saved as " + map_name)

def main():
    parser = argparse.ArgumentParser(description='Map Editor')
    parser.add_argument('--cell_size', '-c', nargs='?',type=int,default=5, help='Cell size')
    parser.add_argument('--map_name', '-m', nargs='?',type=str,default='my_map.txt', help='Cell size')
    args = parser.parse_args()
    global map_name
    map_name = args.map_name

    root = tk.Tk()
    root.title("Map Editor")
    # print("Input the map size (e.g. 200 200):")
    # map_size = input().split()
    print("Please click on the grid to edit the map")
    app = MapEditor(root,cell_size=args.cell_size)
    root.mainloop()

if __name__ == "__main__":
    main()
