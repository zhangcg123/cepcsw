import numpy as np
import matplotlib.pyplot as plt
import os
import argparse

def extract_x0_values(filename):
    x0_values = []
    counter = 0
    with open(filename, 'r') as file:
        for line in file:
            if '|     0 Average Material' in line:
                counter += 1
                items = [item.strip() for item in line.split('|')]
                x0_value = float(items[1].split()[10])
                x0_values.append(x0_value)
                print("processing: ", counter, " x0: ", x0_value)
    return x0_values

def plot_all_in_one(x0_values, output_file, bins, bias, detector):
    """Plot all charts in one figure"""
    theta_bins = bins + 1
    phi_bins = bins*2 + 1
    range_theta = [bias, theta_bins-bias]
    # Adjust figure size and ratio, increase height to accommodate title
    fig = plt.figure(figsize=(28, 8))
    
    # Adjust spacing between subplots and top margin
    gs = fig.add_gridspec(1, 3, width_ratios=[1.2, 1, 1], wspace=0.25, top=0.85)
    
    # Add three subplots
    ax1 = fig.add_subplot(gs[0])  # Heat map
    ax2 = fig.add_subplot(gs[1])  # Phi projection
    ax3 = fig.add_subplot(gs[2])  # Theta projection
    
    # Draw heat map
    X0_matrix = np.array(x0_values).reshape(theta_bins, phi_bins)[range_theta[0]:range_theta[1], :]
    phi = np.linspace(-180, 180, phi_bins)
    theta = np.linspace(range_theta[0]*180/theta_bins, range_theta[1]*180/theta_bins, range_theta[1]-range_theta[0])
    THETA, PHI = np.meshgrid(theta, phi)
    im = ax1.pcolormesh(THETA, PHI, X0_matrix.T, shading='auto', cmap='viridis')
    cbar = plt.colorbar(im, ax=ax1)
    cbar.set_label('X/X0', fontsize=12)
    ax1.set_title('Material Budget Distribution', fontsize=12, pad=10)
    ax1.set_xlabel('Theta (angle)', fontsize=10)
    ax1.set_ylabel('Phi (angle)', fontsize=10)
    
    # Draw phi projection
    phi_projection = np.sum(X0_matrix, axis=0) / (range_theta[1]-range_theta[0])
    ax2.plot(phi, phi_projection, 'b-', linewidth=2)
    ax2.fill_between(phi, phi_projection, alpha=0.3)
    ax2.set_title('Projection along Phi', fontsize=12, pad=10)
    ax2.set_xlabel('Phi (degree)', fontsize=10)
    ax2.set_ylabel('Average X/X0', fontsize=10)
    ax2.grid(True, linestyle='--', alpha=0.7)
    ax2.spines['top'].set_visible(False)
    ax2.spines['right'].set_visible(False)
    
    # Draw theta projection
    theta_projection = np.sum(X0_matrix, axis=1) / phi_bins
    ax3.plot(theta, theta_projection, 'r-', linewidth=2)
    ax3.fill_between(theta, theta_projection, alpha=0.3)
    ax3.set_title('Projection along Theta', fontsize=12, pad=10)
    ax3.set_xlabel('Theta (degree)', fontsize=10)
    ax3.set_ylabel('Average X/X0', fontsize=10)
    ax3.grid(True, linestyle='--', alpha=0.7)
    ax3.spines['top'].set_visible(False)
    ax3.spines['right'].set_visible(False)
    
    # Adjust main title position
    plt.suptitle('Material Budget Analysis for {}'.format(detector), 
                fontsize=16,  # Increase font size
                y=0.95)  # Adjust title vertical position
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=600, bbox_inches='tight')
    plt.show()

def main():
    parser = argparse.ArgumentParser(description='Process material scan data and generate plots.')
    parser.add_argument('--input_file', help='Path to the input text file')
    parser.add_argument('--output_file', default='material_budget.png', help='Path to the output png file')
    parser.add_argument('--detector', default='all_world', help='Detector name')
    parser.add_argument('--bins', type=int, default=100, help='theta bins is "bins"+1, phi bins is "bins"*2+1, (default: 100)')
    parser.add_argument('--bias', type=int, default=0, help='Bias value for theta range (default: 0)')
    args = parser.parse_args()

    # all_world  Coil  Ecal  FTD  Hcal  Lumical  Muon  OTK  ParaffinEndcap  SIT  TPC  VXD
    # bias detectors: BeamPipe onlyTracker
    # Extract detector name from filename
    x0_values = extract_x0_values(args.input_file)
    plot_all_in_one(x0_values, args.output_file, args.bins, args.bias, args.detector)

if __name__ == '__main__':
    main() 