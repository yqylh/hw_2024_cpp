import pygame
import numpy as np
import argparse
import sys

help = ["定义:","0 : 障碍物","1 : 陆地 (通行区域)","2 : 海洋","3 : 泊位","4 : 机器人出生点","按s保存地图"]
map_name = "my_map.txt"

BLOCK_COLOR = (156, 140, 128) 
TEXT_COLOR =  (255, 255, 255)
EARTH_COLOR = (201, 223, 153)
OCEAN_COLOR = (206, 222, 254)
ROBOT_COLOR = (0, 0, 255)
SHIP_COLOR =  (251, 241, 51)
PORT_COLOR =  (190, 193, 198)
ITEM_COLOR =  (249, 139, 228)

color_map = [BLOCK_COLOR, EARTH_COLOR, OCEAN_COLOR, PORT_COLOR, ROBOT_COLOR, SHIP_COLOR]

class CResult:
    def __init__(self) -> None:
        self.result = np.zeros((200, 200), dtype=np.int8)
        self.last_result = np.zeros((200, 200), dtype=np.int8)
    
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
    def __init__(self, x, y, width, height, text):
        self.rect = pygame.Rect(x, y, width, height)
        self.text = text
        self.color = (200, 200, 200)  # 灰色按钮
        self.text_color = (0, 0, 0)  # 黑色文本
        self.font = pygame.font.Font("llt.ttf", 32)

    def draw(self, screen):
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
            Button(self.screen_width - 190, 450, 180, 40, "障碍物"),
            Button(self.screen_width - 190, 500, 180, 40, "陆地"),
            Button(self.screen_width - 190, 550, 180, 40, "海洋"),
            Button(self.screen_width - 190, 600, 180, 40, "泊位"),
            Button(self.screen_width - 190, 650, 180, 40, "机器人"),
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
            if self.now_type !=3:
                rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
                pygame.draw.rect(self.screen, self.rgb_to_fill(color), rect)
                Result.result[x][y] = self.now_type
                # Result.update(x, y, self.now_type)
            else:
                Result.update_last_result()
                for i in range(4):
                    for j in range(4):
                        if 0 <= x + i - 1 < self.map_width and 0 <= y + j - 1 < self.map_height:
                            rect = pygame.Rect((x + i - 1) * self.cell_size, (y + j - 1) * self.cell_size, self.cell_size, self.cell_size)
                            pygame.draw.rect(self.screen, self.rgb_to_fill(color), rect)
                            Result.result[x + i - 1][y + j - 1] = self.now_type
                

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
            self.update_cell(x, y, BLOCK_COLOR)
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
                rect = pygame.Rect(self.start_pos, (self.end_pos[0] - self.start_pos[0], self.end_pos[1] - self.start_pos[1]))
                if rect.width > 0 and rect.height > 0:
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
            button.color = (200, 200, 200)
            if self.now_type == self.buttons2.index(button):
                button.color = (0, 255, 0)
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

            self.screen.fill(self.rgb_to_fill(BLOCK_COLOR))  # 用背景色填充屏幕
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
        for line in to_save_result:
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

def load_map(file):
    lines = []
    with open(file, 'r') as map_file:
        for line in map_file:
            lines.append(line.strip())
    map_height = len(lines)
    map_width = len(lines[0])
    result = np.zeros((map_width, map_height), dtype=np.int8)
    for i, line in enumerate(lines):
        for j, grid_content in enumerate(line):
            if grid_content == '#':
                result[j][i] = 0
            if grid_content == '.':
                result[j][i] = 1
            if grid_content == '*':
                result[j][i] = 2
            if grid_content == 'B':
                result[j][i] = 3
            if grid_content == 'A':
                result[j][i] = 4
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