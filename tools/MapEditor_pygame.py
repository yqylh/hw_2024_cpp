import pygame
import numpy as np
import tkinter
from tkinter import messagebox
import argparse
import sys

help = ["定义:","0 : 障碍物","1 : 陆地 (通行区域)","2 : 海洋","3 : 泊位","4 : 机器人出生点","按s保存地图"]
map_name = "my_map.txt"

# 定义颜色
OBSTACLE_COLOR = (156, 140, 128)  # 障碍物
LAND_COLOR = (201, 223, 153)  # 空地 (陆地)
OCEAN_COLOR = (206, 222, 254)  # 海洋
PORT_COLOR = (190, 193, 198)  # 泊位
# 机器人颜色已废弃
MAIN_ROAD_COLOR = (255, 235, 207)  # 陆地主干道
MAIN_WATERWAY_COLOR = (156, 202, 254)  # 海洋主航道
ROBOT_PURCHASE_BLOCK_COLOR = (255, 140, 0)  # 机器人购买地块，同时该地块也是主干道
SHIP_PURCHASE_BLOCK_COLOR = (0, 0, 255)  # 船舶购买地块，同时该地块也是主航道
DOCKING_AREA_COLOR = (203, 196, 255)  # 靠泊区
LAND_SEA_TRAFFIC_COLOR = (216, 188, 109)  # 海陆立体交通地块
MAIN_LAND_SEA_TRAFFIC_COLOR = (158, 200, 136)  # 海陆立体交通地块，同时为主干道和主航道
DELIVERY_POINT_COLOR = (147, 112, 219)  # 交货点
SHIP_CENTER_COLOR = (90, 93, 98)  # 船泊位中心点

# 创建 color_map，按照索引与地图区块类型的映射关系
color_map = [
    OBSTACLE_COLOR,  # 0 - 障碍
    LAND_COLOR,  # 1 - 空地 (陆地)
    OCEAN_COLOR,  # 2 - 海洋
    PORT_COLOR,  # 3 - 泊位
    (0, 0, 0),  # 4 - 机器人颜色已废弃，使用占位颜色
    MAIN_ROAD_COLOR,  # 5 - 陆地主干道
    MAIN_WATERWAY_COLOR,  # 6 - 海洋主航道
    ROBOT_PURCHASE_BLOCK_COLOR,  # 7 - 机器人购买地块
    SHIP_PURCHASE_BLOCK_COLOR,  # 8 - 船舶购买地块
    DOCKING_AREA_COLOR,  # 9 - 靠泊区
    LAND_SEA_TRAFFIC_COLOR,  # 10 - 海陆立体交通地块
    MAIN_LAND_SEA_TRAFFIC_COLOR,  # 11 - 海陆立体交通地块，同时为主干道和主航道
    DELIVERY_POINT_COLOR,  # 12 - 交货点
    SHIP_CENTER_COLOR, # 13  # 船泊位中心点
]

BLACK_COLOR = (0, 0, 0)

class CResult:
    def __init__(self) -> None:
        self.result = np.zeros((200, 200), dtype=np.int8) + 2
        self.last_result = np.zeros((200, 200), dtype=np.int8) + 2
    
    def update(self, x, y, value):
        self.last_result = self.result.copy()
        self.result[x][y] = value

    # @property
    # def result(self):
    #     return self.result
    
    def update_last_result(self):
        self.last_result = self.result.copy()

    def undo(self):
        self.result = self.last_result.copy()

Result = CResult()

class Button:
    def __init__(self, x, y, width, height, text,color=(200, 200, 200)):
        self.if_pass = False if width != 0 else True
        self.rect = pygame.Rect(x, y, width, height)
        self.text = text
        self.color = color  # 按钮当前颜色
        self.set_color = color  # 按钮原始颜色
        self.text_color = (0, 0, 0)  # 黑色文本
        self.font = pygame.font.Font("llt.ttf", 32)

    def draw(self, screen):
        if not self.if_pass:
            pygame.draw.rect(screen, self.color, self.rect)
            text_surf = self.font.render(self.text, True, self.text_color)
            text_rect = text_surf.get_rect(center=self.rect.center)
            screen.blit(text_surf, text_rect)

    def is_clicked(self, event):
        return self.rect.collidepoint(event.pos)

class MapEditor:
    def __init__(self, map_height=200, map_width=200, cell_size=6):
        pygame.init()
        self.screen_width = map_width * cell_size + 200
        self.screen_height = map_height * cell_size
        self.screen = pygame.display.set_mode((self.screen_width, self.screen_height))
        self.clock = pygame.time.Clock()

        self.font = pygame.font.Font("llt.ttf", 32)

        self.map_height = map_height
        self.map_width = map_width
        self.cell_size = cell_size
        self.choose_type = 0  # 0 for single selection, 1 for box selection
        self.now_type = 0
        self.now_color = color_map[self.now_type]

        self.start_x, self.start_y = 0, 0
        self.end_x, self.end_y = 0, 0

        self.buttons = [
            Button(self.screen_width - 190, 20, 180, 40, "直线"),
            Button(self.screen_width - 190, 70, 180, 40, "框选"),
            Button(self.screen_width - 190, 120, 180, 40, "单击"),
            Button(self.screen_width - 190, 170, 180, 40, "保存"),
        ]
        self.buttons2 = [
            Button(self.screen_width - 190, 450, 180, 40, "障碍物",     color = color_map[0]),
            Button(self.screen_width - 190, 500, 180, 40, "陆地",       color = color_map[1]),
            Button(self.screen_width - 190, 550, 180, 40, "海洋",       color = color_map[2]),
            Button(self.screen_width - 190, 600, 180, 40, "泊位",       color = color_map[3]),
            Button(0, 0, 0, 0, "机器人-废弃", color = color_map[4]),
            Button(self.screen_width - 190, 750, 180, 40, "主干道",     color = color_map[5]),
            Button(self.screen_width - 190, 800, 180, 40, "主航道",     color = color_map[6]),
            Button(self.screen_width - 190, 650, 180, 40, "机器人购买", color = color_map[7]),
            Button(self.screen_width - 190, 700, 180, 40, "船舶购买",   color = color_map[8]),
            Button(self.screen_width - 190, 850, 180, 40, "靠泊区",     color = color_map[9]),
            Button(self.screen_width - 190, 900, 180, 40, "海陆立体交通",color = color_map[10]),
            Button(self.screen_width - 190, 950, 180, 40, "主干海陆立体",color = color_map[11]),
            Button(self.screen_width - 190, 1000, 180, 40, "交货点",    color = color_map[12]),
            Button(self.screen_width - 190, 1050, 180, 40, "船中心点",    color = color_map[13]),
        ]
        self. txts_pos = (self.screen_width - 190, 170 + 40)
        self.log_messages = []

        self.start_x, self.start_y = -1, -1

        self.mouse_down = False
        self.start_pos = None
        self.end_pos = None

    def rgb_to_fill(self, rgb):
        return pygame.Color(*rgb)

    def draw_grid(self):
        for x in range(self.map_width):
            for y in range(self.map_height):
                rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
                pygame.draw.rect(self.screen, self.rgb_to_fill(color_map[Result.result[x][y]]), rect, 0)  # Change 1 to 0 for filled rects
                pygame.draw.rect(self.screen, pygame.Color('gray'), rect, 1) #绘制网格

    def update_cell(self, x, y, color):
        if 0 <= x < self.map_width and 0 <= y < self.map_height:
            if self.now_type != 3 and self.now_type != 13:
                rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
                pygame.draw.rect(self.screen, self.rgb_to_fill(color), rect)
                Result.result[x][y] = self.now_type
                # Result.update(x, y, self.now_type)
            else:
                Result.update_last_result()
                for i in range(9):
                    for j in range(6):
                        now_x = x + i - 3
                        now_y = y + j - 2
                        if 0 <= now_x < self.map_width and 0 <= now_y < self.map_height:
                            # rect = pygame.Rect((now_x) * self.cell_size, (now_y) * self.cell_size, self.cell_size, self.cell_size)
                            # pygame.draw.rect(self.screen, self.rgb_to_fill(color), rect)
                            Result.result[now_x][now_y] = 9
                for i in range(3):
                    for j in range(2):
                        now_x = x + i
                        now_y = y + j
                        if 0 <= now_x < self.map_width and 0 <= now_y < self.map_height:
                            Result.result[now_x][now_y] = 3
                Result.result[x][y] = 13
                self.now_type = 13
                print("泊位已绘制")

    def handle_mouse_event(self, event):
        for button in self.buttons:
            if button.is_clicked(event):
                if button.text == "直线":
                    self.choose_type = 1
                elif button.text == "框选":
                    self.choose_type = 2
                elif button.text == "单击":
                    self.choose_type = 0
                elif button.text == "保存":
                    save_map()
                return
        for button in self.buttons2:
            if button.is_clicked(event):
                self.now_type = self.buttons2.index(button)
                self.now_color = color_map[self.now_type]
                return

        x, y = event.pos[0] // self.cell_size, event.pos[1] // self.cell_size
        if event.button == 1:  # Left click
            if self.choose_type == 0:
                if x < self.map_width and y < self.map_height:
                    self.update_cell(x, y, self.now_color)
                    Result.result[x][y] = self.now_type
                # Result.update(x, y, self.now_type)
            # elif self.choose_type == 2:
            #     if self.start_x != -1 and self.start_y != -1:
            #         self.draw_box(self.start_x, self.start_y, x, y)
            #         self.start_x, self.start_y = -1, -1
            #     else:
            #         self.start_x, self.start_y = x, y
                
        elif event.button == 3:  # Right click
            self.update_cell(x, y, OBSTACLE_COLOR)
            Result.result[x][y] = 0
            # Result.update(x, y, 0)

    def draw_box(self, start_x, start_y, end_x, end_y):
        Result.update_last_result()
        for i in range(min(start_x, end_x), max(start_x, end_x)):
            for j in range(min(start_y, end_y), max(start_y, end_y)):
                if 0 <= i < self.map_width and 0 <= j < self.map_height:
                    self.update_cell(i, j, self.now_color)
                    Result.result[i][j] = self.now_type

    def draw_line(self, start_x, start_y, end_x, end_y):
        Result.update_last_result()
        dx = abs(end_x - start_x)
        dy = -abs(end_y - start_y)
        sx = 1 if start_x < end_x else -1
        sy = 1 if start_y < end_y else -1
        err = dx + dy  # error value e_xy
        while True:
            self.update_cell(start_x, start_y, self.now_color)  # 这里假设 update_cell 方法会根据坐标更新屏幕上的像素
            Result.result[start_x][start_y] = self.now_type  # 更新结果数组
            if start_x == end_x and start_y == end_y:
                break
            e2 = 2 * err
            if e2 >= dy:  # e_xy+e_x > 0
                err += dy
                start_x += sx
            if e2 <= dx:  # e_xy+e_y < 0
                err += dx
                start_y += sy
        


    def draw_log(self,txts):
        font = pygame.font.Font("llt.ttf", 24)
        for i, lines in enumerate(txts):
            text_surf = font.render(lines, True, (0, 0, 0))
            self.screen.blit(text_surf, (self.txts_pos[0], self.txts_pos[1] + i * 30))

    def handle_keyboard_event(self, event):
        if event.key == pygame.K_s:
            save_map()
        elif event.key == pygame.K_e:
            self.choose_type = 1 if self.choose_type == 0 else 0
        elif event.key == pygame.K_q:
            pygame.quit()
            sys.exit()
        elif pygame.K_0 <= event.key <= pygame.K_4:
            self.now_type = event.key - pygame.K_0
            self.now_color = color_map[self.now_type]
        elif event.key == pygame.K_z and (event.mod & pygame.KMOD_CTRL):
            Result.undo()

    def handle_mouse_down_2(self, pos):
        self.mouse_down = True
        pos_t0 = pos[0] // self.cell_size
        pos_t1 = pos[1] // self.cell_size
        # print(pos_t0, pos_t1,pos[0], pos[1])
        pos = (pos_t0 * self.cell_size, pos_t1 * self.cell_size)
        self.start_pos = pos

    def handle_mouse_up_2(self, pos):
        if self.end_pos is not None:
            if self.choose_type == 1:
                self.draw_line(self.start_pos[0] // self.cell_size, self.start_pos[1] // self.cell_size, self.end_pos[0] // self.cell_size, self.end_pos[1] // self.cell_size)
            else:
                self.draw_box(self.start_pos[0] // self.cell_size, self.start_pos[1] // self.cell_size, self.end_pos[0] // self.cell_size, self.end_pos[1] // self.cell_size)
        self.mouse_down = False
        self.start_pos = None
        self.end_pos = None

    def handle_mouse_move(self, pos):
        if self.mouse_down:
            self.end_pos = pos

    def draw_selection_box(self):
        if self.mouse_down and self.start_pos and self.end_pos:
            if self.choose_type == 1:
                color = self.now_color + (128,)
                pygame.draw.line(self.screen, color, (self.start_pos[0], self.start_pos[1]), (self.end_pos[0], self.end_pos[1]), 1)
            else:   
                x = min(self.start_pos[0], self.end_pos[0])
                y = min(self.start_pos[1], self.end_pos[1])
                width = abs(self.end_pos[0] - self.start_pos[0])
                height = abs(self.end_pos[1] - self.start_pos[1])

                # 创建 Rect 对象
                rect = pygame.Rect(x, y, width, height)
                surface = pygame.Surface((rect.width, rect.height), pygame.SRCALPHA)
                surface.fill((255, 255, 255, 128))  # 半透明白色
                self.screen.blit(surface, rect.topleft)

    def draw_button(self):
        for button in self.buttons:
            button.color = (200, 200, 200)
            if button.text == "直线" and self.choose_type == 1:
                button.color = (0, 255, 0)
            elif button.text == "框选" and self.choose_type == 2:
                button.color = (0, 255, 0)
            elif button.text == "单击" and self.choose_type == 0:
                button.color = (0, 255, 0)
            button.draw(self.screen)
        for button in self.buttons2:
            button.color = button.set_color
            if self.now_type == self.buttons2.index(button):
                rect = pygame.Rect(button.rect.x - 5, button.rect.y - 5, button.rect.width + 10, button.rect.height + 10)
                surface = pygame.Surface((rect.width, rect.height), pygame.SRCALPHA)
                surface.fill((0, 255, 0, 128))  # 半透明绿色
                self.screen.blit(surface, rect.topleft)
            button.draw(self.screen)


    def run(self):
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                elif event.type == pygame.MOUSEBUTTONDOWN:
                    self.handle_mouse_event(event)
                    self.handle_mouse_down_2(event.pos)
                elif event.type == pygame.MOUSEBUTTONUP:
                    self.handle_mouse_up_2(event.pos)
                elif event.type == pygame.KEYDOWN:
                    self.handle_keyboard_event(event)
                elif event.type == pygame.MOUSEMOTION:
                    self.handle_mouse_move(event.pos)

            self.screen.fill(self.rgb_to_fill(OBSTACLE_COLOR))  # 用背景色填充屏幕
            self.draw_grid()
            self.draw_button()
            self.draw_log(help)
            self.draw_selection_box()
            pygame.display.flip()
            self.clock.tick(60)

def save_map():
    print("Saving map as " + map_name)
    with open(map_name, 'w') as map_file:
        to_save_result = Result.result.T
        center_list = []
        T_count = 0
        R_count = 0
        S_count = 0
        print(len(to_save_result))
        for x, line in enumerate(to_save_result):
            for y, grid_content in enumerate(line):
                if grid_content == 0:
                    map_file.write('#')
                if grid_content == 1:
                    map_file.write('.')
                if grid_content == 2:
                    map_file.write('*')
                if grid_content == 3 or grid_content == 13:
                    map_file.write('B')
                if grid_content == 4:
                    map_file.write('A')
                if grid_content == 5:
                    map_file.write('>')
                if grid_content == 6:
                    map_file.write('~')
                if grid_content == 7:
                    map_file.write('R')
                    R_count += 1
                if grid_content == 8:
                    map_file.write('S')
                    S_count += 1
                if grid_content == 9:
                    map_file.write('K')
                if grid_content == 10:
                    map_file.write('C')
                if grid_content == 11:
                    map_file.write('c')
                if grid_content == 12:
                    map_file.write('T')
                    T_count += 1
                if grid_content == 13:
                    center_list.append([y, x])
            map_file.write('\n')
        map_file.write(f"{len(center_list)}\n")
        for centerId, center in enumerate(center_list):
            map_file.write(f"{center[1]} {center[0]} 0 2\n")
    print("Map saved as " + map_name)
    if T_count == 0:
        messagebox.showinfo('警告!', '未设置交货点')
    if R_count == 0:
        messagebox.showinfo('警告!', '未设置机器人购买点')
    if S_count == 0:
        messagebox.showinfo('警告!', '未设置船舶购买点')

def load_map(file):
    lines = []
    with open(file, 'r') as map_file:
        for line in map_file:
            lines.append(line.strip())
    map_height = len(lines)
    map_width = len(lines[0])
    result = np.zeros((200, 200), dtype=np.int8)
    for i, line in enumerate(lines):
        if i >= 200:
            break
        for j, grid_content in enumerate(line):
            if grid_content == '#':
                result[j][i] = 0    #障碍
            if grid_content == '.':
                result[j][i] = 1    #空地
            if grid_content == '*':
                result[j][i] = 2    #海洋
            if grid_content == 'B':
                result[j][i] = 3    #泊位
            if grid_content == 'A':
                result[j][i] = 4    #机器人,废弃
            if grid_content == '>':
                result[j][i] = 5    #陆地主干道
            if grid_content == '~':
                result[j][i] = 6    #海洋主干道
            if grid_content == 'R':
                result[j][i] = 7    #机器人购买地块，同时该地块也是主干道
            if grid_content == 'S':
                result[j][i] = 8    #船舶购买地块，同时该地块也是主航道
            if grid_content == 'K':
                result[j][i] = 9    #靠泊区
            if grid_content == 'C':
                result[j][i] = 10   #海陆立体交通地块
            if grid_content == 'c':
                result[j][i] = 11   #海陆立体交通地块，同时为主干道和主航道
            if grid_content == 'T':
                result[j][i] = 12   #交货点
            if j >= 200:
                break
    return result
    

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Map Editor')
    parser.add_argument('--cell_size', '-c', nargs='?',type=int,default=5, help='Cell size')
    parser.add_argument('--map_name', '-m', nargs='?',type=str,default='my_map.txt', help='Cell size')
    parser.add_argument('--load', '-l', nargs='?',type=str,default=None, help='Load map')
    args = parser.parse_args()

    if args.load:
        Result.result = load_map(args.load)

    map_name = args.map_name

    editor = MapEditor()
    editor.run()