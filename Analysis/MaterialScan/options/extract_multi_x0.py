import numpy as np
import matplotlib.pyplot as plt
import os
import argparse

def extract_x0_values(filename):
    """Extract x0 values from a single file"""
    x0_values = []
    counter = 0
    with open(filename, 'r') as file:
        for line in file:
            if '|     0 Average Material' in line:
                counter += 1
                items = [item.strip() for item in line.split('|')]
                x0_value = float(items[1].split()[10])
                x0_values.append(x0_value)
                print(f"Processing file {filename}: ", counter, " x0: ", x0_value)
    return x0_values

def process_multiple_files(file_list, bins, bias):
    """Process multiple files and return their x0 matrices"""
    theta_bins = bins + 1
    phi_bins = bins*2 + 1
    range_theta = [bias, theta_bins-bias]
    
    matrices = []
    for file in file_list:
        x0_values = extract_x0_values(file)
        x0_matrix = np.array(x0_values).reshape(theta_bins, phi_bins)[range_theta[0]:range_theta[1], :]
        matrices.append(x0_matrix)
    
    return matrices

def plot_accumulated_projections(matrices, output_file, bins, bias, labels):
    """Plot accumulated projections"""
    theta_bins = bins + 1
    phi_bins = bins*2 + 1
    range_theta = [bias, theta_bins-bias]
    
    # Create angle arrays
    phi = np.linspace(-180, 180, phi_bins)
    theta = np.linspace(range_theta[0]*180/theta_bins, range_theta[1]*180/theta_bins, range_theta[1]-range_theta[0])
    
    # Create figure
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(20, 8))
    
    # Accumulation data
    accumulated_theta = np.zeros_like(theta)
    accumulated_phi = np.zeros_like(phi)
    
    colors = plt.cm.viridis(np.linspace(0, 1, len(matrices)))
    
    # Plot phi projection
    for i, matrix in enumerate(matrices):
        phi_projection = np.sum(matrix, axis=0) / (range_theta[1]-range_theta[0])
        accumulated_phi += phi_projection
        ax1.fill_between(phi, accumulated_phi, accumulated_phi - phi_projection, 
                        label=labels[i], color=colors[i], alpha=0.6)
    
    # Plot theta projection
    for i, matrix in enumerate(matrices):
        theta_projection = np.sum(matrix, axis=1) / phi_bins
        accumulated_theta += theta_projection
        ax2.fill_between(theta, accumulated_theta, accumulated_theta - theta_projection,
                        label=labels[i], color=colors[i], alpha=0.6)
    
    # Set phi projection plot
    ax1.set_title('Accumulated Projection along Phi', fontsize=14, pad=10)
    ax1.set_xlabel('Phi (degree)', fontsize=12)
    ax1.set_ylabel('Accumulated X/X0', fontsize=12)
    ax1.grid(True, linestyle='--', alpha=0.7)
    ax1.legend(fontsize=10)
    ax1.spines['top'].set_visible(False)
    ax1.spines['right'].set_visible(False)
    
    # Set theta projection plot
    ax2.set_title('Accumulated Projection along Theta', fontsize=14, pad=10)
    ax2.set_xlabel('Theta (degree)', fontsize=12)
    ax2.set_ylabel('Accumulated X/X0', fontsize=12)
    ax2.grid(True, linestyle='--', alpha=0.7)
    ax2.legend(fontsize=10)
    ax2.spines['top'].set_visible(False)
    ax2.spines['right'].set_visible(False)
    
    plt.suptitle('Detector Material Budget Accumulation Analysis', fontsize=16, y=0.95)
    plt.tight_layout()
    
    plt.savefig(output_file, dpi=600, bbox_inches='tight')
    plt.show()

def main():
    parser = argparse.ArgumentParser(description='Process multiple material scan data and generate accumulated distribution plots.')
    parser.add_argument('--input_files', nargs='+', required=True,
                        help='List of input files in order')
    parser.add_argument('--labels', nargs='+', required=True,
                        help='Labels for each input file')
    parser.add_argument('--output_file', default='accumulated_material_budget.png',
                        help='Output PNG file path')
    parser.add_argument('--bins', type=int, default=100,
                        help='theta bins is "bins"+1, phi bins is "bins"*2+1, (default: 100)')
    parser.add_argument('--bias', type=int, default=0,
                        help='Bias value for theta range (default: 0)')
    
    args = parser.parse_args()
    
    if len(args.input_files) != len(args.labels):
        raise ValueError("Number of input files must match number of labels!")
    
    # Process all input files
    matrices = process_multiple_files(args.input_files, args.bins, args.bias)
    
    # Plot accumulated graphs
    plot_accumulated_projections(matrices, args.output_file, args.bins, args.bias, args.labels)

if __name__ == '__main__':
    main()