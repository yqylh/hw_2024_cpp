import cv2
import numpy as np
import sys

EARTH=-1
OCEAN=-2
BLOCK=-3
ROBOT=10
SHIP=20


grid_interval = 0.5
scale_factor = 100
# gridSize = 100
# stageSize = 0.4
# robotSize = 0.45
# robotItemSize = 0.53

BLOCK_COLOR = (156, 140, 128)
TEXT_COLOR =  (255, 255, 255)
EARTH_COLOR = (201, 223, 153)
OCEAN_COLOR = (206, 222, 254)
ROBOT_COLOR = (0, 0, 255)
SHIP_COLOR =  (251, 241, 51)
PORT_COLOR =  (190, 193, 198)
ITEM_COLOR =  (249, 139, 228)

def read_map(map_filename, map_size=200):
    
    grid = [[0] * map_size for i in range(map_size)]
    robot_init_pos = []
    ship_init_pos = []
    
    with open(map_filename, 'r') as map_file:
        lines = map_file.readlines()
        for line_cnt, line_content in enumerate(lines):
            if line_cnt >= map_size: continue
            for row_cnt, grid_content in enumerate(line_content):
                if row_cnt >= map_size: continue
                if grid_content is '.':
                    grid[line_cnt][row_cnt] = EARTH
                if grid_content is '*':
                    grid[line_cnt][row_cnt] = OCEAN
                if grid_content is '#':
                    grid[line_cnt][row_cnt] = BLOCK
                if grid_content is "A":
                    grid[line_cnt][row_cnt] = EARTH
                    robot_init_pos.append([line_cnt, row_cnt])
                if grid_content is "B":
                    grid[line_cnt][row_cnt] = OCEAN
                    ship_init_pos.append([line_cnt, row_cnt])
    
    return grid, robot_init_pos, ship_init_pos

def plot_grid(map_grid, plot_target):
    for i, line in enumerate(map_grid):
        now_y = int(i * grid_interval * scale_factor)
        for j, tar in enumerate(line):
            now_x = int(j * grid_interval * scale_factor)
            if tar == EARTH:
                pt1 = (now_x, now_y)
                pt2 = (int(now_x + grid_interval * scale_factor), int(now_y + grid_interval * scale_factor))
                cv2.rectangle(plot_target, pt1, pt2, EARTH_COLOR, thickness=-1)
            if tar == OCEAN:
                pt1 = (now_x, now_y)
                pt2 = (int(now_x + grid_interval * scale_factor), int(now_y + grid_interval * scale_factor))
                cv2.rectangle(plot_target, pt1, pt2, OCEAN_COLOR, thickness=-1)
            if tar == BLOCK:
                pt1 = (now_x, now_y)
                pt2 = (int(now_x + grid_interval * scale_factor), int(now_y + grid_interval * scale_factor))
                cv2.rectangle(plot_target, pt1, pt2, BLOCK_COLOR, thickness=-1)

def plot_port(plot_target, port_pos, port_size):
    for pos in port_pos:
        pt1 = (int(pos[1] * grid_interval * scale_factor), int(pos[0] * grid_interval * scale_factor))
        pt2 = (int((pos[1] + port_size) * grid_interval * scale_factor), int((pos[0] + port_size) * grid_interval * scale_factor))
        cv2.rectangle(plot_target, pt1, pt2, PORT_COLOR, thickness=-1)


# item is a circle and the item_time will be transfer to alpha with range (0, 1000) to (1.0, 0)
def plot_item(plot_target, item_pos, item_time):
    for i in range(len(item_pos)):
        pos = item_pos[i]
        alpha = 1 - item_time[i] / 1000
        overlay = np.zeros_like(plot_target)
        center = (int((pos[1] + 0.5) * grid_interval * scale_factor), int((pos[0] + 0.5) * grid_interval * scale_factor))
        cv2.circle(overlay, center, int(0.2 * scale_factor), ITEM_COLOR, -1)
        cv2.addWeighted(overlay, alpha, plot_target, alpha, 0, plot_target)

def plot_drone(map_grid, ship_pos, ship_size, robot_pos, robot_size):
    for pos in ship_pos:
        pt1 = (int(pos[1] * grid_interval * scale_factor), int(pos[0] * grid_interval * scale_factor))
        pt2 = (int((pos[1] + ship_size[1]) * grid_interval * scale_factor), int((pos[0] + ship_size[0]) * grid_interval * scale_factor))
        cv2.rectangle(map_grid, pt1, pt2, SHIP_COLOR, thickness=-1)
    
    # robot is a circle
    for pos in robot_pos:
        center = (int((pos[1] + 0.5) * grid_interval * scale_factor), int((pos[0] + 0.5) * grid_interval * scale_factor))
        cv2.circle(map_grid, center, int(0.2 * scale_factor), ROBOT_COLOR, -1)

def draw_img(grid, port_size=(4, 4), ship_size=(2, 4), robot_size=1, port_pos=None, item_pos=None, item_time=None, ship_pos=None, robot_pos=None):
    img = np.zeros((int(grid_interval * scale_factor * len(grid)), int(grid_interval * scale_factor * len(grid[0])), 3), np.uint8)
    plot_grid(grid, img)
    if port_pos is not None:
        plot_port(img, port_pos, port_size)
    if item_pos is not None:
        plot_item(img, item_pos, item_time)
    if ship_pos is not None and robot_pos is not None:
        plot_drone(img, ship_pos, ship_size, robot_pos, robot_size)
    
    print("hi")
    to_show = cv2.resize(img, (1250, 1250))
    cv2.imshow('map', to_show)
    
    
def main(map_id):
    map_filename = "../allMaps/map" + str(map_id) + ".txt"
    map_grid, robot_pos, ship_pos = read_map(map_filename)
    
    draw_img(map_grid, ship_pos=ship_pos, robot_pos=robot_pos)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
    
if __name__ == "__main__":
    main(1)