import pygame
import numpy as np
import argparse
import sys
from tqdm import tqdm
import time

try:
    from IPython.core import debugger
    debug = debugger.Pdb().set_trace
except:
    debug = print

now_robot = -1
now_time = -1
help = ["Replayer测试","当前robot:",str(now_robot),"当前frame:",str(now_time)]
map_name = "my_map.txt"

PATH_LINE_COLOR = (255, 120, 120) + (128,)


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

SHIPSHIFT = [[[0,0],[1,0],[2,0],[0,1],[1,1],[2,1]],\
            [[0,0],[-1,0],[-2,0],[0,-1],[-1,-1],[-2,-1]],\
            [[0,0],[1,0],[0,-1],[1,-1],[0,-2],[1,-2]],\
            [[0,0],[-1,0],[0,1],[-1,1],[0,2],[-1,2]]]

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
    (0,0,240), #14 -  船
    (0,140,240), #14 -  船核心
]

BLACK_COLOR = (0, 0, 0)

class CResult:
    def __init__(self) -> None:
        self.result = np.zeros((200, 200), dtype=np.int8)
        self.ori_result = np.zeros((200, 200), dtype=np.int8)
        self.belongMap = np.zeros((200, 200), dtype=np.int8)
        self.update_xys = []
        self.last_update_xyzs = []
    
    def updata_results(self,x,y,v):
        self.update_xys.append([x,y])
        self.result[x,y] = v
    
    def re_result(self):
        self.last_update_xyzs = self.update_xys
        self.update_xys = []
        self.result = self.ori_result.copy()

Result = CResult()
robotpos = np.zeros((15000, 30, 2), dtype=np.int32) - 1 #x,y
robotstat = np.ones((15000, 30, 1), dtype=np.int32)
shippos = np.zeros((15000, 20, 3), dtype=np.int32) - 1 # x,y,direction
# robotstat = np.ones((15000, 20, 1), dtype=np.int32)
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
        self.screen_height = map_height * cell_size + 75
        self.screen = pygame.display.set_mode((self.screen_width, self.screen_height), pygame.SRCALPHA)
        self.clock = pygame.time.Clock()

        self.font = pygame.font.Font("llt.ttf", 32)

        self.map_height = map_height
        self.map_width = map_width
        self.cell_size = cell_size
        self.robot_cnt = 0
        self.ship_cnt = 0

        self.buttons = [
            Button(self.screen_width - 190, 20, 180, 40, "前进一帧"),
            Button(self.screen_width - 190, 70, 180, 40, "后退一帧"),
            Button(self.screen_width - 190, 120, 180, 40, "上个robot"),
            Button(self.screen_width - 190, 170, 180, 40, "下个robot"),
        ]
        self.buttons2 = [
            Button(self.screen_width - 190, 900, 180, 40, "自动播放/暂停"),
            Button(self.screen_width - 190, 950, 180, 40, "快进"),
            Button(self.screen_width - 190, 1000, 180, 40, "快退"),
        ]
        self.txts_pos = (self.screen_width - 190, 170 + 40)
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
        self.path_change = False
        self.draw_belong = True
        
        self.line_surface = pygame.Surface((self.screen_width, self.screen_height), pygame.SRCALPHA)
        self.belong_surface = pygame.Surface((self.screen_width, self.screen_height), pygame.SRCALPHA)
        self.belong_surface.fill((0, 0, 0, 0))
        
        belong_color_list = ['#946ABB', '#F68F35', '#237EBD', '#E87FC8', '#2DA631', '#59CBD3', '#95655C', '#828282']
        
        for x in range(self.map_width):
            for y in range(self.map_height):
                if Result.belongMap[x][y] != 0:
                    color = belong_color_list[Result.belongMap[x][y] - 1]
                    rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
                    pygame.draw.rect(self.belong_surface, color, rect, 0)
                    
        self.belong_surface.set_alpha(32)
        
        self.reats_witoutmap1 = (map_width * cell_size,0,200,map_height * cell_size)
        self.reats_witoutmap2 = (0,map_height * cell_size,map_width * cell_size,75)

    def rgb_to_fill(self, rgb):
        return pygame.Color(*rgb)

    def draw_grid(self):
        for x in range(self.map_width):
            for y in range(self.map_height):
                rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
                pygame.draw.rect(self.screen, self.rgb_to_fill(color_map[Result.result[x][y]]), rect, 0)  # Change 1 to 0 for filled rects
                pygame.draw.rect(self.screen, pygame.Color('gray'), rect, 1) #绘制网格
    
    def draw_changed_grid(self):
        for pos in Result.last_update_xyzs:
            x = pos[0]
            y = pos[1]
            rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
            pygame.draw.rect(self.screen, self.rgb_to_fill(color_map[Result.ori_result[x][y]]), rect, 0)
            pygame.draw.rect(self.screen, pygame.Color('gray'), rect, 1) #绘制网格
        for pos in Result.update_xys:
            x = pos[0]
            y = pos[1]
            rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
            pygame.draw.rect(self.screen, self.rgb_to_fill(color_map[Result.result[x][y]]), rect, 0)  # Change 1 to 0 for filled rects
            pygame.draw.rect(self.screen, pygame.Color('gray'), rect, 1) #绘制网格

    def upspeed(self):
        if self.auto_play_speed > 0:
            self.auto_play_speed = 2 * self.auto_play_speed if self.auto_play_speed <16 else self.auto_play_speed
        else:
            self.auto_play_speed = int(self.auto_play_speed / 2) if self.auto_play_speed != -1 else 1
    
    def downspeed(self):
        if self.auto_play_speed > 1:
            self.auto_play_speed = int(self.auto_play_speed / 2 )
        elif 0.0625 < self.auto_play_speed <= 1:
            self.auto_play_speed = self.auto_play_speed / 2
        elif self.auto_play_speed == 0.0625:
            self.auto_play_speed = -1
        else:
            self.auto_play_speed = 2 * self.auto_play_speed if self.auto_play_speed > -16 else self.auto_play_speed

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
                    now_robot = now_robot - 1 if now_robot > 0 else self.robot_cnt
                    now_robot = now_robot if now_robot <= self.robot_cnt else 0
                    self.choose_path(now_robot)
                elif button.text == "下个robot":
                    now_robot = now_robot + 1 if now_robot < self.robot_cnt else 0
                    now_robot = now_robot if now_robot >= 0 else self.robot_cnt
                    self.choose_path(now_robot)
        for button in self.buttons2:
            if button.is_clicked(event):
                if button.text == "自动播放/暂停":
                    self.auto_play = True if self.auto_play == False else False
                    if self.auto_play_speed < 0:
                         self.auto_play_speed = 1
                elif button.text == "快进":
                    self.upspeed()
                elif button.text == "快退":
                    self.downspeed()

        # x, y = event.pos[0] // self.cell_size, event.pos[1] // self.cell_size
        pro = self.get_progress_from_click(event.pos[0], event.pos[1])
        if pro is not None:
            now_time = int(15000 * pro)

        if event.button == 1:  # Left click
            pass
                
        elif event.button == 3:  # Right click
            pass

    def draw_log(self,txts):
        font = pygame.font.Font("llt.ttf", 24)
        for i, lines in enumerate(txts):
            text_surf = font.render(lines, True, (0, 0, 0))
            self.screen.blit(text_surf, (self.txts_pos[0], self.txts_pos[1] + i * 30))

    def draw_progress_bar(self, progress):
    # 进度条的位置和尺寸
        x = 50
        y = self.screen_height - 50
        width = self.screen_width - 100
        height = 25

        # 绘制进度条背景
        background_color = (200, 200, 200)  # 灰色背景
        pygame.draw.rect(self.screen, background_color, (x, y, width, height))

        # 绘制进度条前景（填充部分）
        fill_color = (0, 255, 0)  # 绿色填充
        filled_width = int(width * progress)  # 根据进度计算填充宽度
        pygame.draw.rect(self.screen, fill_color, (x, y, filled_width, height))

    def get_progress_from_click(self, x, y):
        # 进度条的位置和尺寸
        bar_x = 50
        bar_y = self.screen_height - 50
        bar_width = self.screen_width - 100
        bar_height = 25

        # 判断点击是否在进度条范围内
        if bar_x <= x <= bar_x + bar_width and bar_y <= y <= bar_y + bar_height:
            # 计算点击位置相对于进度条开始位置的偏移量，并计算比例
            relative_x = x - bar_x
            progress = relative_x / bar_width
            return progress
        else:
            return None  # 或者根据你的需求返回0

    def handle_keyboard_event(self, event):
        global now_time
        global now_robot
        if event.key == pygame.K_a or event.key == pygame.K_LEFT:
            if not self.auto_play:
                now_time = max(0,now_time-1)
            else:
                self.downspeed()
        elif event.key == pygame.K_d or event.key == pygame.K_RIGHT:
            if not self.auto_play:
                now_time = min(15000,now_time+1)
            else:
                self.upspeed()
                    
        elif event.key == pygame.K_w or event.key == pygame.K_UP:
            now_robot = now_robot - 1 if now_robot > 0 else self.robot_cnt
            now_robot = now_robot if now_robot <= self.robot_cnt else 0
            self.choose_path(now_robot)
        elif event.key == pygame.K_s or event.key == pygame.K_DOWN:
            now_robot = now_robot + 1 if now_robot < self.robot_cnt else 0
            now_robot = now_robot if now_robot >= 0 else self.robot_cnt
            self.choose_path(now_robot)
        elif event.key == pygame.K_SPACE:
            self.auto_play = True if self.auto_play == False else False
        elif event.key == pygame.K_b:
            self.draw_belong = True if self.draw_belong == False else False

    def draw_robots(self):
        self.robot_cnt = 0
        self.ship_cnt = 0
        Result.re_result()
        if 0< now_time < 15000:
            trobot_pos = robotpos[now_time]
            tship_pos = shippos[now_time]
            for i in range(30): #画机器人
                if trobot_pos[i][0] != -1:
                    self.robot_cnt += 1
                    if robotstat[now_time][i] != 2:
                        Result.updata_results(trobot_pos[i][1],trobot_pos[i][0], 4)  
                    else:
                        Result.updata_results(trobot_pos[i][1],trobot_pos[i][0], 13) 
                else:
                    break
            for i in range(20): #画船
                if tship_pos[i][0] != -1:
                    self.ship_cnt += 1
                    hx = tship_pos[i][1]
                    hy = tship_pos[i][0]
                    for shift in SHIPSHIFT[tship_pos[i][2]]:
                        Result.updata_results(hx + shift[0],hy + shift[1] , 14)
                    Result.updata_results(hx,hy,15)
                else:
                    break
                    
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
            if time - now_time >= -200:
                deltime = time - now_time
            else:
                deltime = 100000
            if deltime < min_time and robotpath_path[i] != []:
                if len(robotpath_path[i]) + deltime > 0 and deltime < 0: #正在走的路
                    that_path_id = i
                    that_frame = time
                    break
                elif deltime < 0:
                    i += 1
                    continue
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
            print("\nstart time: ",self.path_start_time)
            print("life time:",life_time)
            print("deadtime:",self.path_dead_time)
            print("now time",now_time)
            # print("path",path)
            self.path_change = True

    def draw_gds(self):
        for i in range(self.goods_alive_from,gds.shape[0]):
            good = gds[i]
            y,x,start_time,dead_time,val = good[0] ,good[1] ,good[2] ,good[3] ,good[4]
            tdead_time = start_time + 1000
            rect = pygame.Rect(x * self.cell_size, y * self.cell_size, self.cell_size, self.cell_size)
            if tdead_time < now_time or dead_time < now_time:
                pygame.draw.rect(self.screen, self.rgb_to_fill(color_map[Result.ori_result[x][y]]), rect, 0)
                pygame.draw.rect(self.screen, pygame.Color('gray'), rect, 1) #绘制网格
            elif start_time > now_time:
                continue
            else:
                pygame.draw.rect(self.screen, self.rgb_to_fill((255,238 - int(val),255)), rect, 0)  # Change 1 to 0 for filled rects
                pygame.draw.rect(self.screen, pygame.Color('gray'), rect, 1) #绘制网格
    
    def draw_path(self,path):
        #带透明度,但更慢
        if self.path_change:
            self.draw_grid()
            self.line_surface.fill((0, 0, 0, 0))
            plen = len(path)
            if plen == 0: return
            i = 1
            start_point = (path[0][1] * self.cell_size ,path[0][0] * self.cell_size)
            while(i<plen):
                end_point = (path[i][1] * self.cell_size,path[i][0] * self.cell_size)
                try:
                    pygame.draw.line(self.line_surface, PATH_LINE_COLOR, start_point, end_point, 3)
                except:
                    print(start_point, end_point)
                
                start_point = end_point
                i+=1
            self.screen.blit(self.line_surface, (0, 0))
            self.path_change = False
        else:
            self.screen.blit(self.line_surface, (0, 0))     

    def draw_path_fast(self,path):
        if self.path_change:
            self.draw_grid()
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
                self.path_change = False
        else:
            pass

    def handle_mouse_down_2(self, pos):
        self.mouse_down = True
        pos_t0 = pos[0] // self.cell_size
        pos_t1 = pos[1] // self.cell_size
        # print(pos_t0, pos_t1,pos[0], pos[1])
        pos = (pos_t0 * self.cell_size, pos_t1 * self.cell_size)
        self.start_pos = pos
        print(pos_t1, pos_t0)

    def handle_mouse_up_2(self, pos):
        self.mouse_down = False
        self.start_pos = None
        self.end_pos = None

    def handle_mouse_move(self, pos):
        global now_time
        if self.mouse_down:
            self.end_pos = pos
            pro = self.get_progress_from_click(pos[0], pos[1])
            if pro is not None:
                now_time = int(15000 * pro)

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
        next_frame=0
        self.screen.fill(self.rgb_to_fill(OBSTACLE_COLOR))  # 用背景色填充屏幕
        self.draw_grid()
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
            
            # self.screen.fill(self.rgb_to_fill(OBSTACLE_COLOR),self.reats_witoutmap1)
            pygame.draw.rect(self.screen, self.rgb_to_fill(OBSTACLE_COLOR),self.reats_witoutmap2)
            pygame.draw.rect(self.screen, self.rgb_to_fill(OBSTACLE_COLOR),self.reats_witoutmap1)
            self.draw_gds()
            self.draw_changed_grid()
            self.draw_robots()
            if self.path_start_time <= now_time:
                self.draw_path(self.path)
            elif self.path_start_time - now_time > 2: #还有很久才开始?换!
                self.choose_path(now_robot)
            
            # 有bug:
            if self.draw_belong:
                self.screen.blit(self.belong_surface, (0, 0))
            
            
            self.draw_button()
            self.draw_progress_bar(now_time/15000)
            help = ["Replayer测试","","快退有bug","","当前robot:",str(now_robot),"当前frame:",str(now_time),\
                           "当前倍速:",str(self.auto_play_speed),\
                            "","","FPS:{:.2f}".format(self.clock.get_fps())]
            self.draw_log(help)
            # self.draw_selection_box()
            pygame.display.flip()

            self.clock.tick(60)

            if self.auto_play:
                next_frame += self.auto_play_speed
                now_time = min(max(now_time + int(next_frame),0),14999)
                next_frame = next_frame - int(next_frame)
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

def load_belong_berth(file):
    lines = []
    with open(file, 'r') as map_file:
        for line in map_file:
            lines.append(line.strip())
    if len(lines) == 0:
        return None
    map_height = len(lines)
    map_width = len(lines[0])
    result = np.zeros((map_width, map_height), dtype=np.int8)
    for i, line in enumerate(lines):
        for j, grid_content in enumerate(line):
            result[j][i] = int(grid_content)
    print("map belonging success")
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

def load_ship_pos(file_name):
    with open(file_name, 'r') as file:
        lines = file.readlines()
    lin_len = len(lines)
    i = 0
    id = -1
    ship_id = 0
    while(i < lin_len):
        x,y = lines[i].strip().split()
        x = int(x)
        y = int(y)
        if x == -1000:
            id = y
            ship_id = 0
            i += 1
            continue
        shippos[id,ship_id,0] = x
        shippos[id,ship_id,1] = y
        dir,stat = lines[i+1].strip().split()
        shippos[id,ship_id,2] = int(dir)
        i += 2
        ship_id += 1
    print("load ship pos success!")

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
    # print(len(robotpath_path))
    # print(len(robotpath_frame))
    # print(len(robotpath_robot_id))
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
    parser.add_argument('--load', '-l','-m', nargs='?',type=str,default='../allMaps/map1.txt', help='Load map')

    parser.add_argument('--robot_path_file','-ra', nargs='?',type=str,default='../log/counter.txt_robot_path.txt')
    parser.add_argument('--robot_pos_file','-ro', nargs='?',type=str,default='../log/counter.txt_robot_pos.txt')
    parser.add_argument('--goods_pos_file','-rg', nargs='?',type=str,default='../log/counter.txt_gds.txt')
    parser.add_argument('--ship_pos_file','-rs', nargs='?',type=str,default='../log/counter.txt_ship_pos.txt')
    
    args = parser.parse_args()

    if args.load:
        map = load_map(args.load)
        Result.result = map
        Result.ori_result = map.copy()
        
    Result.belongMap = load_belong_berth("../log/berthbelong.txt")

    load_gds(args.goods_pos_file)
    load_pos(args.robot_pos_file)
    load_ship_pos(args.ship_pos_file)
    load_path(args.robot_path_file)

    editor = MapEditor()
    editor.run()