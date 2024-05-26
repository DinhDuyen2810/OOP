import os
import json
import numpy as np
import scipy.stats as stats
import sys

# Đường dẫn đến thư mục hiện tại
current_directory = os.path.dirname(__file__)

# Đường dẫn đến file input.json
file_path = os.path.join(current_directory, '..', '..', 'data', 'input.json')

# Đọc dữ liệu từ file input.json
with open(file_path, 'r') as input_file:
    data = json.load(input_file)

# Lấy thông tin về phân phối chuẩn từ file JSON
distribution_data = data.get("eventDistribution", {}).get("distribution", {}).get("normal", {})
n = distribution_data.get("samples")
k = distribution_data.get("numberOfValues")
min_value = distribution_data.get("minValue")
max_value = distribution_data.get("maxValue")

# Thiết lập mã hóa UTF-8 cho đầu ra
sys.stdout = open(sys.stdout.fileno(), mode='w', encoding='utf-8', buffering=1)

# Khởi tạo biến đếm số lượng mẫu tuân theo phân phối chuẩn
count_normal_samples = 0

# Sinh ra ngẫu nhiên các giá trị của mẫu tuân theo phân bố chuẩn
mean = (min_value + max_value) / 2
std = (max_value - min_value) / 6

allEvents = []

while count_normal_samples < 5:
    sample = np.random.normal(mean, std, n)
    sample = np.round(sample, 1)
    
    # Tính toán giá trị p_value để kiểm tra xem mẫu dữ liệu có phải là phân bố chuẩn hay không
    stat, p_value = stats.shapiro(sample)
    
    # Kiểm tra mức ý nghĩa thống kê
    alpha = 0.05
    if p_value >= alpha:
        allEvents.append(sample)
        count_normal_samples += 1

# Đường dẫn đến file output
output_file_path = os.path.join(current_directory, '..', '..', 'data', 'outputpy.txt')

# Mở file output để ghi dữ liệu
with open(output_file_path, 'w') as output_file:
    # Ghi thông tin về dữ liệu vào file
    for event in allEvents:
        output_file.write(' '.join(map(str, event)) + '\n')
