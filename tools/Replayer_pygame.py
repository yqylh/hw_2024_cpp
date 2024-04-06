import pygame
import numpy as np
import argparse
import sys
from tqdm import tqdm

try:
    from IPython.core import debugger
    debug = debugger.Pdb().set_trace
except:
    debug = print

now_robot = -1
now_time = -1
help = ["Replayer测试","当前robot:",str(now_robot),"当前frame:",str(now_time)]
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
    (197,106,119), #13 - 标记带货的机器人
]

BLACK_COLOR = (0, 0, 0)

class CResult:
    def __init__(self) -> None:
        self.result = np.zeros((200, 200), dtype=np.int8)
        self.ori_result = np.zeros((200, 200), dtype=np.int8)

Result = CResult()
robotpos = np.zeros((15000, 20, 2), dtype=np.int32) - 1
robotstat = np.ones((15000, 20, 1), dtype=np.int32)
gds = []
gds_dead = []

robotpath_path = []
robotpath_path2 = []
robotpath_frame = []
robotpath_robot_id = []


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
    global now_time
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

        self.buttons = [
            Button(self.screen_width - 190, 20, 180, 40, "前进一帧"),
            Button(self.screen_width - 190, 70, 180, 40, "后退一帧"),
            Button(self.screen_width - 190, 120, 180, 40, "上个robot"),
            Button(self.screen_width - 190, 170, 180, 40, "下个robot"),
        ]
        self.buttons2 = [
            Button(self.screen_width - 190, 900, 180, 40, "自动播放/暂停"),
            Button(self.screen_width - 190, 950, 180, 40, "快进"),
        ]
        self. txts_pos = (self.screen_width - 190, 170 + 40)
        self.log_messages = []

        self.path = []
        self.path_dead_time = 0
        self.path_start_time = 0

        self.mouse_down = False
        self.start_pos = None
        self.end_pos = None
        self.auto_play = False
        self.auto_play_speed = 1
        self.goods_alive_from = 0

    def rgb_to_fill(self, rgb):
        return pygame.Color(*rgb)

    def draw_grid(self):
        for x in range(self.map_width):
            for y in range(self.map_height):
                rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
                pygame.draw.rect(self.screen, self.rgb_to_fill(color_map[Result.result[x][y]]), rect, 0)  # Change 1 to 0 for filled rects
                pygame.draw.rect(self.screen, pygame.Color('gray'), rect, 1) #绘制网格

    def handle_mouse_event(self, event):
        global now_time
        global now_robot
        for button in self.buttons:
            if button.is_clicked(event):
                if button.text == "前进一帧":
                    now_time = min(15000,now_time+1)
                elif button.text == "后退一帧":
                    now_time = max(0,now_time-1)
                elif button.text == "上个robot":
                    now_robot = max(0,now_robot-1)
                    self.choose_path(now_robot)
                elif button.text == "下个robot":
                    now_robot = min(20,now_robot+1)
                    self.choose_path(now_robot)
        for button in self.buttons2:
            if button.is_clicked(event):
                if button.text == "自动播放/暂停":
                    self.auto_play = True if self.auto_play == False else False
                elif button.text == "快进":
                    self.auto_play_speed = 2 * self.auto_play_speed if self.auto_play_speed <16 else 1

        x, y = event.pos[0] // self.cell_size, event.pos[1] // self.cell_size

        if event.button == 1:  # Left click
            pass
                
        elif event.button == 3:  # Right click
            pass

    def draw_log(self,txts):
        font = pygame.font.Font("llt.ttf", 24)
        for i, lines in enumerate(txts):
            text_surf = font.render(lines, True, (0, 0, 0))
            self.screen.blit(text_surf, (self.txts_pos[0], self.txts_pos[1] + i * 30))

    def handle_keyboard_event(self, event):
        global now_time
        if event.key == pygame.K_a or event.key == pygame.K_LEFT:
            now_time = max(0,now_time-1)
        elif event.key == pygame.K_d or event.key == pygame.K_RIGHT:
            now_time = min(15000,now_time+1)
        elif event.key == pygame.K_w or event.key == pygame.K_UP:
            pass
        elif event.key == pygame.K_s or event.key == pygame.K_DOWN:
            pass
        elif event.key == pygame.K_SPACE:
            self.auto_play = True if self.auto_play == False else False

    def draw_robots(self):
        Result.result = Result.ori_result.copy()
        if now_time > 0:
            trobot_pos = robotpos[now_time]
            for i in range(20):
                if trobot_pos[i][0] != -1:
                    Result.result[trobot_pos[i][1],trobot_pos[i][0]] = 4 if robotstat[now_time][i] != 2 else 13

    def choose_path(self,robot_id = None):
        global now_time
        global now_robot
        min_time = 10000
        that_frame = -1
        that_path_id = -1
        i=0
        path = []
        for time in robotpath_frame:
            if robot_id is not None:
                if robotpath_robot_id[i] != robot_id:
                    i += 1
                    continue
            if time - now_time >= -5:
                deltime = time - now_time
            else:
                deltime = 100000
            if deltime < min_time and robotpath_path[i] != []:
                min_time = deltime
                that_frame = time
                that_path_id = i
            i += 1

        if that_frame != -1:
            #  = that_frame 
            self.path_start_time = robotpath_frame[that_path_id]
            now_robot = robotpath_robot_id[that_path_id]
            path = robotpath_path[that_path_id]
            life_time = len(path)
            self.path_dead_time = self.path_start_time + life_time
            self.path = path
            # debug()
            print("start time: ",self.path_start_time)
            print("life time:",life_time)
            print("deadtime:",self.path_dead_time)
            print("path",path)

    def draw_gds(self):
        for i in range(self.goods_alive_from,gds.shape[0]):
            good = gds[i]
            y,x,start_time,dead_time,val = good[0] ,good[1] ,good[2] ,good[3] ,good[4]
            tdead_time = start_time + 1000
            if tdead_time < now_time:
                self.goods_alive_from += 1
                continue
            if dead_time < now_time:
                continue
            if start_time > now_time:
                continue
            rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
            surface = pygame.Surface((rect.width, rect.height), pygame.SRCALPHA)
            surface.fill((255, 128, 128, val))  # 半透明白色
            self.screen.blit(surface, rect.topleft)

    def draw_path(self,path):
        plen = len(path)
        if plen == 0: return
        i = 1
        start_point = (path[0][1] * self.cell_size ,path[0][0] * self.cell_size)
        while(i<plen):
            end_point = (path[i][1] * self.cell_size,path[i][0] * self.cell_size)
            try:
                pygame.draw.line(self.screen, (255, 0, 0), start_point, end_point, 3)
            except:
                print(start_point, end_point)
            start_point = end_point
            i+=1

    def handle_mouse_down_2(self, pos):
        self.mouse_down = True
        pos_t0 = pos[0] // self.cell_size
        pos_t1 = pos[1] // self.cell_size
        # print(pos_t0, pos_t1,pos[0], pos[1])
        pos = (pos_t0 * self.cell_size, pos_t1 * self.cell_size)
        self.start_pos = pos

    def handle_mouse_up_2(self, pos):
        self.mouse_down = False
        self.start_pos = None
        self.end_pos = None

    def handle_mouse_move(self, pos):
        if self.mouse_down:
            self.end_pos = pos

    def draw_button(self):
        for button in self.buttons:
            button.color = (200, 200, 200)
            button.draw(self.screen)
        for button in self.buttons2:
            button.color = button.set_color
            button.draw(self.screen)

    def run(self):
        global now_time
        global now_robot
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
            self.draw_robots()
            if self.path_start_time <= now_time:
                self.draw_path(self.path)
            self.draw_gds()
            self.draw_button()
            help = help = ["Replayer测试","当前robot:",str(now_robot),"当前frame:",str(now_time),\
                           "当前倍速:",str(self.auto_play_speed)]
            self.draw_log(help)
            # self.draw_selection_box()
            pygame.display.flip()
            self.clock.tick(60)
            if self.auto_play:
                now_time += 1 * self.auto_play_speed
                if self.path_dead_time < now_time:
                    self.choose_path(now_robot)

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
    print("map load success")
    return result

def load_pos(file_name):
    with open(file_name, 'r') as file:
        lines = file.readlines()
    lin_len = len(lines)
    i = 0
    id = -1
    robot_id = 0
    while(i < lin_len):
        x,y = lines[i].strip().split()
        x = int(x)
        y = int(y)
        if x == -1000:
            if y != -2:
                id = y
                robot_id = 0
                i += 1
                continue
            elif y == -2:
                i += 1
                x,y = lines[i].strip().split()
                robot_id = int(x)
                stat = int(y)
                robotstat[id,robot_id] = stat
                if stat == 2: #拿了货物,要把gds对应位置货物删掉
                    gx = robotpos[id,robot_id,0]
                    gy = robotpos[id,robot_id,1]
                    gdsid = (gds[:,0] == gx) & (gds[:,1] == gy)
                    gds[gdsid,3] = id
                i+=1
                continue
        robotpos[id,robot_id,0] = x
        robotpos[id,robot_id,1] = y
        robotstat[id,robot_id] = robotstat[id-1,robot_id]
        i += 1
        robot_id += 1
    print("load robot pos success!")

def load_path(file_name):
    with open(file_name, 'r') as file:
        lines = file.readlines()
    lin_len = len(lines)
    i = 0
    id = -1
    robot_id = 0
    path = []
    while(i < lin_len):
        x,y = lines[i].strip().split()
        x = int(x)
        y = int(y)
        if x == -1000:
            if y != 2:
                if path != []:
                    robotpath_path.append(path)
                    robotpath_frame.append(id)
                    robotpath_robot_id.append(robot_id)
                i += 1
                x,y = lines[i].strip().split()
                robot_id = int(x)
                id = int(y)
                path = []
                i += 1
                continue
            else:
                i += 1
                x,y = lines[i].strip().split()
                x = int(x)
                y = int(y)
        path.append([x,y])
        i += 1
    print("load robot path success!")
    print(len(robotpath_path))
    print(len(robotpath_frame))
    print(len(robotpath_robot_id))
    # for i in range(len(robotpath_robot_id)):
    #     print(robotpath_frame[i],robotpath_robot_id[i])

def load_gds(file_name):
    global gds
    with open(file_name, 'r') as file:
        lines = file.readlines()
    lin_len = len(lines)
    frame = -1
    i = 0
    while (i < lin_len - 1):
        x,y = lines[i].strip().split()
        x = int(x)
        y = int(y)
        if x == -1000:
            if y > 0:
                frame = int(y)
                i+=1
            continue
        _,val = lines[i+1].strip().split()
        val = int(val)
        gds.append([x,y,frame,frame + 1000,val])
        i+=2
    gds = np.asarray(gds)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Map Editor')
    parser.add_argument('--cell_size', '-c', nargs='?',type=int,default=5, help='Cell size')
    parser.add_argument('--map_name', '-m', nargs='?',type=str,default='my_map.txt', help='Cell size')
    parser.add_argument('--robot_path_file','-ra', nargs='?',type=str,default='../log/counter.txt_robot_path.txt')
    parser.add_argument('--robot_pos_file','-ro', nargs='?',type=str,default='../log/counter.txt_robot_pos.txt')
    parser.add_argument('--goods_pos_file','-rg', nargs='?',type=str,default='../log/counter.txt_gds.txt')
    parser.add_argument('--load', '-l', nargs='?',type=str,default='../allMaps/map1.txt', help='Load map')
    args = parser.parse_args()

    if args.load:
        map = load_map(args.load)
        Result.result = map
        Result.ori_result = map.copy()

    map_name = args.map_name
    load_gds(args.goods_pos_file)
    load_pos(args.robot_pos_file)
    load_path(args.robot_path_file)

    editor = MapEditor()
    editor.run()