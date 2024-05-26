import os
import sys
import json
import numpy as np
import scipy.stats as stats
import io

# Đặt mã hóa đầu ra mặc định là UTF-8
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# Lấy đường dẫn tới thư mục chứa file Python hiện tại
current_directory = os.path.dirname(os.path.abspath(__file__))

# Đường dẫn đến file input.json
file_path = os.path.join(current_directory, '..', '..', 'data', 'input.json')

# Đọc dữ liệu từ file JSON
with open(file_path, 'r', encoding='utf-8') as file:
    data = json.load(file)

# Số lượng mẫu, tổng giá trị của các mẫu, cận dưới và cận trên của các giá trị mẫu
distribution_data = data.get("timeBetweenEventsDistribution", {}).get("distribution", {}).get("normal", {})
n = int(distribution_data.get("samples"))
k = int(distribution_data.get("numberOfValues"))
min_value = float(distribution_data.get("minValue"))
max_value = float(distribution_data.get("maxValue"))

# Sinh ra ngẫu nhiên các giá trị của mẫu tuân theo phân bố chuẩn
mean = (min_value + max_value) / 2  # Trung bình của phân bố chuẩn
std = (max_value - min_value) / 6  # Độ lệch chuẩn của phân bố chuẩn

# Thiết lập mức ý nghĩa thống kê
alpha = 0.05  # Mức ý nghĩa thống kê

while True:
    sample = np.random.normal(mean, std, n)  # Mẫu dữ liệu tuân theo phân bố chuẩn
    sample = np.round(sample, 0)

    # Tính toán giá trị p_value để kiểm tra xem mẫu dữ liệu có phải là phân bố chuẩn hay không
    stat, p_value = stats.shapiro(sample)  # Tính toán giá trị thống kê và giá trị p_value

    # Kiểm tra p_value và thoát khỏi vòng lặp nếu mẫu tuân theo phân bố chuẩn
    if p_value >= alpha:
        break

# Đường dẫn đến file outputpy.txt
output_file_path = os.path.join(current_directory, '..', '..', 'data', 'outputpy.txt')

# Ghi kết quả vào tệp, xóa nội dung cũ và chèn vào nội dung mới
with open(output_file_path, 'w', encoding='utf-8') as output_file:
    output_file.write(' '.join(map(str, sample)) + '\n')

