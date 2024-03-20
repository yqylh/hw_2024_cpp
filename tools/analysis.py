import re
from collections import defaultdict

from cv2 import Mat

def analyze_judger_output_v3(file_path):
    # 初始化统计字典
    duration_stats = {
        '0~6ms': 0,
        '7~12ms': 0,
        '13~16ms': 0,
        '异常情况': 0
    }
    # 分别存储13~16ms和异常情况的时间戳
    timestamps_13_to_16 = []
    timestamps_abnormal = []

    with open(file_path, 'r') as file:
        for line in file:
            match = re.search(r'step (\d+)\s+(\d+) ms,', line)
            if match:
                timestamp, duration = int(match.group(1)), int(match.group(2))
                # 根据耗时分别统计
                if 0 <= duration <= 6:
                    duration_stats['0~6ms'] += 1
                elif 7 <= duration <= 12:
                    duration_stats['7~12ms'] += 1
                elif 13 <= duration <= 16:
                    duration_stats['13~16ms'] += 1
                    timestamps_13_to_16.append(timestamp)
                else:
                    duration_stats['异常情况'] += 1
                    timestamps_abnormal.append(timestamp)

    # 输出统计结果和相关时间戳
    print("耗时分布情况及特定情况时间戳：")
    for category, count in duration_stats.items():
        print(f"{category}: {count} 次")
        if category == '13~16ms' and count > 0:
            print("  对应时间戳：", ', '.join(map(str, timestamps_13_to_16)))
        elif category == '异常情况' and count > 0:
            print("  对应时间戳：", ', '.join(map(str, timestamps_abnormal)))

def analyze_time_distribution(file_path):
    time_distribution = defaultdict(int)
    total_time = 0
    total_count = 0
    max_time = 0

    with open(file_path, 'r') as file:
        for line in file:
            match = re.search(r'rId=\d+solveGridWithTime time=(\d+)', line)
            if match:
                time = int(match.group(1))
                # 分组统计，每100为一个区间
                group = (time // 100) * 100
                time_distribution[group] += 1
                
                # 计算总时间和次数，用于计算平均值
                total_time += time
                total_count += 1

                # 更新最大值
                if time > max_time:
                    max_time = time

    # 计算平均值
    average_time = total_time / total_count if total_count else 0

    # 输出分组统计结果
    print("Time的分布情况（按100的尺度）：")
    for group in sorted(time_distribution):
        print(f"Time {group}-{group+99}: {time_distribution[group]}次")

    # 输出平均值和最大值
    print(f"\n平均值：{average_time:.2f}")
    print(f"最大值：{max_time}")

def analyze_goods_distribution(file_path):

    with open(file_path, 'r') as file:
        for line in file:
            # target is like 123:tmpTotalGoods:456, what we need is 456
            matchTotal = re.search(r'tmpTotalGoods:(\d+)', line)
            if matchTotal:
                goods = int(matchTotal.group(1))
                print("货物总量", goods)
            matchLeft = re.search(r'leftTotal:(\d+)', line)
            if matchLeft:
                left = int(matchLeft.group(1))
                print("剩余货物", left)

