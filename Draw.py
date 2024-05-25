import matplotlib.pyplot as plt

# tự sửa lại link dẫn đến input
file_path = "C:\\Users\\Admin\\Documents\\Code\\BTL OOP\\output\\input.txt"

def read_input(file_path):
    nodes = {}
    edges = []
    with open(file_path, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if parts[0] == 'n' and len(parts) >= 4:
                node_id = parts[1]
                coords = parts[-1].split(',')  
                if len(coords) == 2:
                    x, y = coords
                    nodes[node_id] = (float(x), float(y))
            elif parts[0] == 'a' and len(parts) >= 5:  
                from_node = parts[1]
                to_node = parts[2]
                weight = parts[-1]
                edges.append((from_node, to_node, float(weight)))
    return nodes, edges

def draw_graph(nodes, edges):
    plt.figure(figsize=(8, 6))  

    for edge in edges:
        start_id, end_id, weight = edge
        start_x, start_y = nodes[start_id]
        end_x, end_y = nodes[end_id]
        dx = end_x - start_x
        dy = end_y - start_y
        plt.arrow(start_x, start_y, dx, dy, head_width=0.2, head_length=0.25, length_includes_head=True, color='red')

        midpoint_x = (start_x + end_x) / 2
        midpoint_y = (start_y + end_y) / 2

    plt.xlabel('X')
    plt.ylabel('Y')
    plt.title('Graph')
    plt.grid(False)  
    plt.ylim(0, max(coord[1] for coord in nodes.values()) + 1)
    plt.xticks(range(int(min([coord[0] for coord in nodes.values()])), int(max([coord[0] for coord in nodes.values()])) + 1))
    plt.yticks(range(int(min([coord[1] for coord in nodes.values()])), int(max([coord[1] for coord in nodes.values()])) + 1))
    plt.gca().invert_yaxis()
    plt.show()


nodes, edges = read_input(file_path)
if nodes and edges:  
    draw_graph(nodes, edges)
else:
    print("No data found in the input file.")


