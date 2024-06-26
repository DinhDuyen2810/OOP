import os
import json
import numpy as np
from scipy import stats
import sys
import io

# Lấy đường dẫn tới thư mục chứa file Python hiện tại
current_directory = os.path.dirname(os.path.abspath(__file__))

# Đường dẫn đến file input.json
file_path = os.path.join(current_directory, '..', '..', 'data', 'input.json')

# Đọc dữ liệu từ file JSON
with open(file_path, 'r', encoding='utf-8') as file:
    data = json.load(file)

# Đặt mã hóa mặc định thành UTF-8
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# Số lượng mẫu, tổng giá trị của các mẫu, cận dưới và cận trên của các giá trị mẫu
distribution_data = data.get("walkability", {}).get("distribution", {}).get("normal", {})
num_samples = int(distribution_data.get("samples"))
total_value = 10000
lower_bound = int(distribution_data.get("lowerBound"))
upper_bound = int(distribution_data.get("upperBound"))

# Tính giá trị trung bình và độ lệch chuẩn
mean = total_value / num_samples

std_dev = (mean - lower_bound) / 3  # Giả sử rằng 99.7% dữ liệu nằm trong khoảng 3 độ lệch chuẩn từ giá trị trung bình

# Tạo ra các mẫu ngẫu nhiên tuân theo phân phối chuẩn
samples = np.random.normal(mean, std_dev, num_samples)

# Chuẩn hóa các mẫu sao cho tổng của chúng bằng với total_value
samples = samples * total_value / np.sum(samples)

# Đảm bảo rằng tất cả các mẫu đều nằm trong khoảng từ lower_bound đến upper_bound
samples = np.clip(samples, lower_bound, upper_bound)
samples = np.round(samples, 0)

# Kiểm tra xem liệu các mẫu này có tuân theo phân phối chuẩn không bằng cách sử dụng kiểm định Shapiro-Wilk
stat, p = stats.shapiro(samples)
alpha = 0.05

# Mở file output để ghi kết quả
output_file_path = os.path.join(current_directory, '..', '..', 'data', 'outputpy.txt')

# Mở tệp với chế độ 'w+' để xóa nội dung cũ và ghi nội dung mới
with open(output_file_path, 'w+', encoding='utf-8') as output_file:
    stat, p = stats.kstest(samples, 'norm')

    values = samples.astype(int)
    max_idx = values.argmax()

    # Hoán đổi phần tử lớn nhất với phần tử đầu tiên
    values[0], values[max_idx] = values[max_idx], values[0]
    output_file.write(' '.join(map(str, values)) + '\n')
