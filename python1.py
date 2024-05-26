import sys
import io
import numpy as np
import scipy.stats as stats
import json
import os

# Lấy đường dẫn tới thư mục chứa file Python hiện tại
current_directory = os.path.dirname(os.path.abspath(__file__))

# Đường dẫn đến file input.json theo cấu trúc mới
file_path = os.path.join(current_directory, '..', '..', 'data', 'input.json')

# Đọc dữ liệu từ file JSON
with open(file_path, 'r') as file:
    data = json.load(file)

# Truy cập các giá trị từ phần JSON
distribution_data = data.get("ageDistribution", {}).get("distribution", {}).get("normal", {})
k = distribution_data.get("numberOfValues")
min_value = distribution_data.get("minValue")
max_value = distribution_data.get("maxValue")
n = data.get("numOfAgents", {}).get("value")

# Đặt mã hóa mặc định thành UTF-8
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

alpha = 0.05  # Mức ý nghĩa thống kê
p_value = 0  # Khởi tạo p_value để bắt đầu vòng lặp

# Sinh ngẫu nhiên các giá trị của mẫu và kiểm tra phân phối chuẩn
while True:
    # Sinh ra ngẫu nhiên các giá trị của mẫu tuân theo phân bố chuẩn
    mean = (min_value + max_value) / 2  # Trung bình của phân bố chuẩn
    std = (max_value - min_value) / 6   # Độ lệch chuẩn của phân bố chuẩn
    sample = np.random.normal(mean, std, n)  # Mẫu dữ liệu tuân theo phân bố chuẩn
    sample = np.clip(sample, min_value, max_value)  # Cắt bớt các giá trị ngoài khoảng [min_value, max_value]
    sample = np.round(sample, 1)

    # Tính toán giá trị p_value để kiểm tra xem mẫu dữ liệu có phải là phân bố chuẩn hay không
    stat, p_value = stats.kstest(sample, 'norm', args=(mean, std))

    # Kiểm tra giá trị p_value để quyết định dừng vòng lặp
    if p_value >= alpha:
        break

# Đường dẫn đến file outputpy.txt theo cấu trúc mới
output_file_path = os.path.join(current_directory, '..', '..', 'data', 'outputpy.txt')

# Ghi kết quả vào file outputpy.txt
with open(output_file_path, 'w', encoding='utf-8') as output_file:
    # In mẫu dữ liệu, các giá trị cách nhau bởi dấu cách
    output_file.write('\n'.join(map(str, sample)) + '\n')
