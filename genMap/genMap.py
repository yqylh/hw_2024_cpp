import cv2
import numpy as np
import sys


gridInterval = 0.5
scaleFactor = 100
gridSize = 100
stageSize = 0.4
robotSize = 0.45
robotItemSize = 0.53

def readPath(fileName):
    # {"id": []}
    robotPath = [[] for i in range(4)]
    with open(fileName, 'r') as pathFile:
        pathLines = pathFile.readlines()
        nowTime = 0
        carry = 0
        origin = 0
        nowId = 0
        for lineCnt, lineContent in enumerate(pathLines):
            if lineCnt % 2 == 0:
                items = lineContent.split(' ')
                nowTime = int(items[0].split('=')[1])
                nowId = int(items[1].split('=')[1])
                if items[2] == "origin": origin = 1
                else: origin = 0
                if int(items[3].split('=')[1]) == 1: carry = 1
                else: carry = 0
            else:
                pointsContents = lineContent.split('->')
                points = []
                for pointContent in pointsContents:
                    if len(pointContent) < 3: continue
                    x, y = pointContent.strip("()").split(",")
                    points.append((float(x), float(y)))
                robotPath[nowId].append((nowTime, origin, carry, points))
    return robotPath

def readMap(mapFileName):
    # grid is a 100 x 100 matrix
    grid = [[0] * 100 for i in range(100)]
    
    with open(mapFileName, 'r') as mapFile:
        mapLines = mapFile.readlines()
        for lineCnt, lineContent in enumerate(mapLines):
            if lineCnt >= 100: continue
            for rowCnt, gridContent in enumerate(lineContent):
                if rowCnt >= 100: continue
                if gridContent.isdigit():
                    grid[lineCnt][rowCnt] = int(gridContent) 
                if gridContent == 'A':
                    grid[lineCnt][rowCnt] = -2
                if gridContent == '#':
                    grid[lineCnt][rowCnt] = -1
    return grid

def realWordToGrid(point):
    realWorld = 50
    totalGridSize = gridSize * gridInterval * scaleFactor
    perGrid = totalGridSize / realWorld
    return (int(point[0] * perGrid), int(totalGridSize - point[1] * perGrid))


def drawMap(mapGrid, plotTarget):
    for i, line in enumerate(mapGrid):
        nowY = int(i * gridInterval * scaleFactor)
        for j, content in enumerate(line):
            nowX = int(j * gridInterval * scaleFactor)
            if content == -1:
                pt1 = (nowX, nowY)
                pt2 = (int(nowX + gridInterval * scaleFactor), int(nowY + gridInterval * scaleFactor))
                color = (0, 0, 0)
                cv2.rectangle(plotTarget, pt1, pt2, color, thickness=-1)
                
    for i, line in enumerate(mapGrid):
        nowY = int(i * gridInterval * scaleFactor)
        for j, content in enumerate(line):
            nowX = int(j * gridInterval * scaleFactor)
            if content > 0:
                pt1 = (nowX, nowY)
                pt2 = (int(nowX + 1.2 * gridInterval * scaleFactor), int(nowY + 1.2 * gridInterval * scaleFactor))
                color = (255, 0, 0)
                cv2.rectangle(plotTarget, pt1, pt2, color, -1)
                
                text = str(content)
                textPos = (int(pt1[0] + 0.08 * scaleFactor), int(pt2[1] - 0.08 * scaleFactor))
                # M = cv2.getRotationMatrix2D(textPos, 90, 1)
                # rotated_text = cv2.warpAffine(img, M, img.shape[:2])
                cv2.putText(plotTarget, text, textPos, cv2.FONT_HERSHEY_SIMPLEX, 0.02 * scaleFactor, (255, 255, 255), int(0.05 * scaleFactor))
    '''
    for i, line in enumerate(mapGrid):
        nowY = int(i * gridInterval * scaleFactor)
        for j, content in enumerate(line):
            nowX = int(j * gridInterval * scaleFactor)
            if content == -2:
                center = (int(nowX + gridInterval * scaleFactor // 2), int(nowY + gridInterval * scaleFactor // 2))
                redius = int(robotSize * scaleFactor)
                color = (0, 0, 255)
                cv2.circle(plotTarget, center, redius, color, thickness=-1)
    '''

def drawPath(robotPath, pathOrigin, carry, plotTarget):
    beginPixle = realWordToGrid(robotPath[0])
    if carry:
        diameter = realWordToGrid((robotItemSize * 2, robotItemSize * 2))[0]
    else:
        diameter = realWordToGrid((robotSize * 2, robotSize * 2))[0]
    
    for i in range(len(robotPath) - 1):
        nowPixle = realWordToGrid(robotPath[i])
        nextPixle = realWordToGrid(robotPath[i + 1])
        
        if pathOrigin == 1:
            color = (255, 255, 0)
        else:
            color = (194, 157, 241)

        # print(robotSize * gridInterval * scaleFactor * 2)
        # print((robotItemSize * gridInterval * scaleFactor * 2))
        cv2.line(plotTarget, nowPixle, nextPixle, color, diameter)
    cv2.circle(plotTarget, beginPixle, int(diameter / 2), (0, 0, 255), thickness=-1)
    if pathOrigin == 1:
        for point in robotPath:
            nowPixel = realWordToGrid(point)
            radius = realWordToGrid((0.25, 0.25))[0]
            pt1 = [nowPixel[0] - radius, nowPixel[1] - radius]
            pt2 = [nowPixel[0] + radius, nowPixel[1] + radius]

            cv2.rectangle(plotTarget, pt1, pt2, (255, 255, 255), thickness=-1)

def drawImage(mapGrid, nowPath, nowRobotId):
    plotTarget = np.zeros((int(gridSize * gridInterval * scaleFactor), int(gridSize * gridInterval * scaleFactor), 3), np.uint8)
    pt1 = (0, 0)
    pt2 = (int(gridSize * gridInterval * scaleFactor), int(gridSize * gridInterval * scaleFactor))
    color = color = (140, 230, 240)
    cv2.rectangle(plotTarget, pt1, pt2, color, thickness=-1)
    
    drawPath(nowPath[3], nowPath[1], nowPath[2], plotTarget)
    drawMap(mapGrid, plotTarget)
    textContent = "robotId=%d, time=%d, carry=%d" % (nowRobotId, nowPath[0], nowPath[2])
    org = (20, 80)
    font = cv2.FONT_HERSHEY_SIMPLEX
    fontScale = 2.5
    color = (0, 0, 255)
    thickness = 8
    cv2.putText(plotTarget, textContent, org, font, fontScale, color, thickness)

    toShow = cv2.resize(plotTarget, (1250, 1250))
    cv2.imshow('map', toShow)

def saveImage(mapGrid, nowPath, nowRobotId):
    plotTarget = np.zeros((int(gridSize * gridInterval * scaleFactor), int(gridSize * gridInterval * scaleFactor), 3), np.uint8)
    pt1 = (0, 0)
    pt2 = (int(gridSize * gridInterval * scaleFactor), int(gridSize * gridInterval * scaleFactor))
    color = color = (140, 230, 240)
    cv2.rectangle(plotTarget, pt1, pt2, color, thickness=-1)
    
    drawPath(nowPath[3], nowPath[1], nowPath[2], plotTarget)
    drawMap(mapGrid, plotTarget)
    textContent = "robotId=%d, time=%d, carry=%d" % (nowRobotId, nowPath[0], nowPath[2])
    org = (20, 80)
    font = cv2.FONT_HERSHEY_SIMPLEX
    fontScale = 2.5
    color = (0, 0, 255)
    thickness = 8
    cv2.putText(plotTarget, textContent, org, font, fontScale, color, thickness)

    cv2.imwrite('saved_image.jpg', plotTarget)

def main(mapId=1):
    robotPaths = readPath('path' + str(mapId) + '.txt')
    # [[(time, origin, carry, [(x, y), (x, y), ...]), ...], ...]
    # [id, [time, origin, carry, [points]], ...]
    mapGrid = readMap('maps/' + str(mapId) + '.txt')
    nowRobotId = 0
    nowPathId = 0
    while True:
        if nowPathId < 0:
            nowPathId += len(robotPaths[nowRobotId])
        nowPathId = nowPathId % len(robotPaths[nowRobotId])
        nowPath = robotPaths[nowRobotId][nowPathId]
        drawImage(mapGrid, nowPath, nowRobotId)
        key = cv2.waitKey(0)

        if key == ord('1'):
            nowRobotId = 0
            nowPath = 0
        elif key == ord('2'):
            nowRobotId = 1
            nowPath = 0
        elif key == ord('3'):
            nowRobotId = 2
            nowPath = 0
        elif key == ord('4'):
            nowRobotId = 3
            nowPath = 0
        elif key == ord('j'):
            nowPathId -= 1
        elif key == ord('k'):
            nowPathId += 1
        elif key == ord('n'):
            nowPathId -= 10
        elif key == ord('m'):
            nowPathId += 10
        elif key == ord('i'):
            nowPathId -= 1000
        elif key == ord('o'):
            nowPathId += 1000
        elif key == 27:
            cv2.destroyAllWindows()
            break
        elif key == ord('s'):
            saveImage(mapGrid, nowPath, nowRobotId)
    
    # plotTarget = cv2.rotate(plotTarget, cv2.ROTATE_90_CLOCKWISE)
    # 
    
if __name__ == "__main__":
    main(sys.argv[1])